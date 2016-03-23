//#define LOG_NDEBUG 0
#define LOG_TAG "tcpStream"

#include <stdlib.h>
#include <CdxTypes.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <CdxSocketUtil.h>
#include <netdb.h> 

#include <CdxStream.h>
#include <CdxAtomic.h>

#include <SmartDnsService.h>

#define SOCKRECVBUF_LEN 512*1024// 262142 (5*1024*1024)
#define closesocket close

typedef struct CdxTcpStreamImpl
{
    CdxStreamT base;
    cdx_int32 ioState;
    cdx_int32 sockRecvBufLen;
    cdx_int32 notBlockFlag;
//    cdx_int32 exitFlag;                               //when close, exit
    cdx_int32 forceStopFlag;
    cdx_int32 sockFd;                                 //socket fd    
    //int eof;                                    //all stream data is read from network    
    cdx_int32 port;
    cdx_char *hostname;
    cdx_atomic_t ref;                           //reference count, for free resource while still blocking.     

    cdx_atomic_t state;
    pthread_mutex_t stateLock;
    
    pthread_cond_t dnsCond;
    pthread_mutex_t dnsMutex;
    int dnsRet;
    struct addrinfo *dnsAI;
	//YUNOS
	cdx_char mTcpIP[100];
    ParserCallback callback;
    void *pUserData;
	int mYunOSstatusCode;
    //add more 
}CdxTcpStreamImplT;

static cdx_void CdxTcpStreamDecRef(CdxStreamT *stream);

typedef struct CdxHttpSendBuffer
{
    void *size;
    void *buf;
}CdxHttpSendBufferT;

enum HttpStreamStateE
{
    TCP_STREAM_IDLE    = 0x00L,
    TCP_STREAM_CONNECTING = 0x01L,
    TCP_STREAM_READING = 0x02L,
    TCP_STREAM_WRITING = 0x03L,
    TCP_STREAM_FORCESTOPPED = 0x04L,
    //TCP_STREAM_CLOSING 
};

static int64_t GetNowUs() 
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000000ll + tv.tv_usec;
}

static cdx_int32 __CdxTcpStreamRead(CdxStreamT *stream, void *buf, cdx_uint32 len)
{
    CdxTcpStreamImplT *impl;
    cdx_int32 ret;
    cdx_int32 recvSize = 0;
    cdx_int32 ioErr;
    cdx_int32 num = 0;
    
    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    if(stream == NULL || buf == NULL || len <= 0)
    {
        CDX_LOGW("check parameter.");
        return -1;
    }
    
	pthread_mutex_lock(&impl->stateLock);
	if(impl->forceStopFlag)
	{
		pthread_mutex_unlock(&impl->stateLock);
		return -2;
	}
    CdxAtomicInc(&impl->ref);
    CdxAtomicSet(&impl->state, TCP_STREAM_READING);
	pthread_mutex_unlock(&impl->stateLock);
	    
    while(impl->notBlockFlag)
    {
        if(impl->forceStopFlag)
        {
            //CDX_LOGV("__CdxTcpStreamRead forceStop.");
            ret = -2;
            goto __exit0;
        }
        ret = CdxSockNoblockRecv(impl->sockFd, buf, len);
        if (ret < 0)
        {
            ioErr = errno;
            if (EAGAIN == ioErr)
            {
                num++;
                usleep(5000);
                if(num < 400) //* notBlockFlag, try 2s at most.
                {
                    continue;
                }
                else
                {
                    ret = 0;
                }
            }
            else
            {
                CDX_LOGE("<%s,%d>recv err(%d): %s", __FUNCTION__, __LINE__, errno, strerror(errno));
                impl->ioState = CDX_IO_STATE_ERROR;
                impl->notBlockFlag = 0;
                ret = -1;
                goto __exit0;
            }
        }
        CDX_LOGV("xxx CdxSockNoblockRecv(%d)", ret);
        impl->notBlockFlag = 0;
        
__exit0:
    	pthread_mutex_lock(&impl->stateLock);
        CdxAtomicSet(&impl->state, TCP_STREAM_IDLE);
        CdxTcpStreamDecRef(stream);
    	pthread_mutex_unlock(&impl->stateLock);

        return ret;
    }

    while((cdx_uint32)recvSize < len)
    {
        if(impl->forceStopFlag)
        {
            CDX_LOGV("__CdxTcpStreamRead forceStop.");
            if(recvSize > 0)
                break;
            else
            {
                recvSize = -2;
                goto __exit1;
            }                
        }
        ret = CdxSockAsynRecv(impl->sockFd, (char *)buf + recvSize, 
                                            len - recvSize, 0, &impl->forceStopFlag);
        if(ret < 0)
        {
            if(ret == -2)
            {
                recvSize = recvSize>0 ? recvSize : -2;
                goto __exit1;
            }
            impl->ioState = CDX_IO_STATE_ERROR;
            CDX_LOGE("__CdxTcpStreamRead error(%d): %s. recvSize(%d)", errno, strerror(errno), recvSize);
            recvSize = -1;
			if(impl->callback)
			{
				impl->mYunOSstatusCode = 3003; //Ali YUNOS invoke info
				impl->callback(impl->pUserData, STREAM_EVT_DOWNLOAD_DOWNLOAD_ERROR, &(impl->mYunOSstatusCode));
			}

            goto __exit1;
        }
        else if(ret == 0)
        {
            //CDX_LOGD("xxx ret = 0.");
            //if(++num >= 100)
            //{
            //    CDX_LOGW("xxx connection closed by peer, fd(%d)", impl->sockFd);
            //    impl->ioState = CDX_IO_STATE_ERROR;
            //    CdxAtomicDec(&impl->ref);
            //    return -1;
            //}
            break;
        }
        else
        {
            recvSize += ret;
        }
    }

__exit1:
    pthread_mutex_lock(&impl->stateLock);
    CdxAtomicSet(&impl->state, TCP_STREAM_IDLE);
    CdxTcpStreamDecRef(stream);
    pthread_mutex_unlock(&impl->stateLock);
    
    return recvSize;
}

static cdx_int32 __CdxTcpStreamGetIOState(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    return impl->ioState;
}
static cdx_int32 __CdxTcpStreamWrite(CdxStreamT *stream, void *buf, cdx_uint32 len)
{
    CdxTcpStreamImplT *impl;
    size_t size = 0;
    ssize_t ret = 0;
    
    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

	pthread_mutex_lock(&impl->stateLock);
	if(impl->forceStopFlag)
	{
		pthread_mutex_unlock(&impl->stateLock);
		return -1;
	}
    CdxAtomicInc(&impl->ref);
    CdxAtomicSet(&impl->state, TCP_STREAM_WRITING);
	pthread_mutex_unlock(&impl->stateLock);

    while(size < len)
    {
        ret = CdxSockAsynSend(impl->sockFd, (char *)buf + size, len - size, 
                                                        0, &impl->forceStopFlag);
        if(ret < 0)
        {
            CDX_LOGE("send failed. error(%d): %s.", errno, strerror(errno));
            break;
        }
        else if(ret == 0)
        {
            break;
        }
        size += ret;
    }
    
    pthread_mutex_lock(&impl->stateLock);
    CdxAtomicSet(&impl->state, TCP_STREAM_IDLE);
    CdxTcpStreamDecRef(stream);
    pthread_mutex_unlock(&impl->stateLock);

    return (size == len) ? 0 : -1;  
}
static cdx_int32 CdxTcpStreamForceStop(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;
    long ref;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);
    
    CDX_LOGV("begin tcp force stop");
    pthread_mutex_lock(&impl->stateLock);
    //if((ref = CdxAtomicRead(&impl->state)) == TCP_STREAM_FORCESTOPPED)
    //{
    //    pthread_mutex_unlock(&impl->stateLock);
    //    return 0;
    //}
    CdxAtomicInc(&impl->ref);
    impl->forceStopFlag = 1;
    pthread_mutex_unlock(&impl->stateLock);
    
    while(((ref = CdxAtomicRead(&impl->state)) != TCP_STREAM_IDLE)/* && 
    		((ref = CdxAtomicRead(&impl->state)) != TCP_STREAM_CONNECTING)*/)
    {
        //CDX_LOGV("xxx state(%ld)", ref);
        usleep(10*1000);
    }

    pthread_mutex_lock(&impl->stateLock);
    //CdxAtomicSet(&impl->state, TCP_STREAM_FORCESTOPPED);
    pthread_mutex_unlock(&impl->stateLock);
    CdxTcpStreamDecRef(stream);
    CDX_LOGV("finish tcp force stop");
    return 0;
}
static cdx_int32 CdxTcpStreamClrForceStop(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    pthread_mutex_lock(&impl->stateLock);
    impl->forceStopFlag = 0;
    CdxAtomicSet(&impl->state, TCP_STREAM_IDLE);
    pthread_mutex_unlock(&impl->stateLock);

    return 0;
}

static cdx_int32 __CdxTcpStreamControl(CdxStreamT *stream, cdx_int32 cmd, void *param)
{
    CdxTcpStreamImplT *impl;
    
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    switch(cmd)
    {
        case STREAM_CMD_READ_NOBLOCK:
        {
            impl->notBlockFlag = 1;
            break;            
        }
        case STREAM_CMD_GET_SOCKRECVBUFLEN:
        {
            *(int *)param = impl->sockRecvBufLen;
            break;
        }
        case STREAM_CMD_SET_FORCESTOP:
        {
            return CdxTcpStreamForceStop(stream);
        }
        case STREAM_CMD_CLR_FORCESTOP:
        {
            return CdxTcpStreamClrForceStop(stream);
        }
		case STREAM_CMD_GET_IP:
		{
			if(impl->mTcpIP[0])
				strcpy((char *)param,impl->mTcpIP);
			break;
		}
        case STREAM_CMD_SET_CALLBACK:
        {
            struct CallBack *cb = (struct CallBack *)param;
            impl->callback = cb->callback;
            impl->pUserData = cb->pUserData;
            break;
        }
        default:
        {
            CDX_LOGE("should not be here.");
            break;
        }
    }
    return 0;
}

static cdx_int32 __CdxTcpStreamClose(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);
    CDX_LOGV("xxx tcp close begin.");
    CdxAtomicInc(&impl->ref);
    
    CdxTcpStreamForceStop(stream);   
    CdxTcpStreamDecRef(stream);
    CdxTcpStreamDecRef(stream);

    return 0;
}
static cdx_void CdxTcpStreamDecRef(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    CdxAtomicDec(&impl->ref);
    if(CdxAtomicRead(&impl->ref) == 0)
    {
        CdxSockDisableTcpKeepalive(impl->sockFd);
        shutdown(impl->sockFd, SHUT_RDWR); 
        closesocket(impl->sockFd);
        pthread_mutex_destroy(&impl->stateLock);
        free(impl);
    }
    return;
}

static void DnsResponeHook(void *userhdr, int ret, struct addrinfo *ai)
{
    CdxTcpStreamImplT *impl = (CdxTcpStreamImplT *)userhdr;

    if (ret == SDS_OK)
    {
        impl->dnsAI = ai;

//		CDX_LOGD("%x%x%x", ai->ai_addr->sa_data[0], ai->ai_addr->sa_data[1], ai->ai_addr->sa_data[2]);
    }
    impl->dnsRet = ret;

    pthread_mutex_lock(&impl->dnsMutex);
    pthread_cond_signal(&impl->dnsCond);
    pthread_mutex_unlock(&impl->dnsMutex);

    return ;
}

static int StartTcpStreamConnect(CdxTcpStreamImplT *pArg)
{
    CdxTcpStreamImplT *impl;
    cdx_int32 ret;
    int64_t start, end;
    struct addrinfo *ai = NULL;

    start = GetNowUs();
    
    impl = pArg;
    CDX_FORCE_CHECK(impl);
    
    //impl->dnsRet = SDSRequest(impl->hostname, impl->port, &ai, impl, DnsResponeHook);
    ret = SDSRequest(impl->hostname, impl->port, &ai, impl, DnsResponeHook);
    if (ret == SDS_OK)
    {
        CDX_FORCE_CHECK(ai);
    }
    else if (ret == SDS_PENDING)
    {
        while (1)
        {
            struct timespec abstime;
                    
            abstime.tv_sec = time(0);
            abstime.tv_nsec = 100000000L;
            
            pthread_mutex_lock(&impl->dnsMutex);
            pthread_cond_timedwait(&impl->dnsCond, &impl->dnsMutex, &abstime); /* wait 100 ms */
            pthread_mutex_unlock(&impl->dnsMutex);

            if (impl->forceStopFlag)
            {
                ai = NULL;
                break;
            }
            
            if (impl->dnsRet == SDS_OK)
            {
                ai = impl->dnsAI;
                break;
            }
            else if (impl->dnsRet != SDS_PENDING)
            {
                ai = NULL;
                break;
            }
            
        }
    
    }

    if (ai == NULL)
    {
        goto err_out;
    }

	//* Ali YUNOS invoke info
	//* get tcp IP in control STREAM_CMD_GET_IP
	struct addrinfo *curInfo;
	struct sockaddr_in *addrInfo;
	curInfo = ai;
	cdx_char ipbufInfo[100];
	addrInfo = (struct sockaddr_in *)curInfo->ai_addr;
	memcpy(impl->mTcpIP,inet_ntop(AF_INET, &addrInfo->sin_addr, ipbufInfo, 100),100);
    do
    {
        ret = CdxSockAsynConnect(impl->sockFd, ai->ai_addr, ai->ai_addrlen, 0, &impl->forceStopFlag);
        if(ret == 0)
        {
            break;
        }
        else if(ret < 0)
        {
            CDX_LOGE("connect failed. error(%d): %s.", errno, strerror(errno));
			if(impl->callback)
			{
				impl->mYunOSstatusCode = 3002; //Ali YUNOS invoke info
				impl->callback(impl->pUserData, STREAM_EVT_DOWNLOAD_DOWNLOAD_ERROR, &(impl->mYunOSstatusCode));
			}
            goto err_out;
        }
        
        if(impl->forceStopFlag == 1)
        {
            CDX_LOGV("force stop connect.");
            goto err_out;
        }
    } while ((ai = ai->ai_next) != NULL);
    
    if (ai == NULL)
    {
        CDX_LOGE("connect failed. error(%d): %s.", errno, strerror(errno));
        goto err_out;
    }
    
    end = GetNowUs();
    //CDX_LOGV("Start tcp time(%lld)", end-start);
    return 0;
    
err_out:
    end = GetNowUs();
    if(errno == 101)
    {
    	CDX_LOGD("errno 101, reconnect");
    	return -2;
    }
    //CDX_LOGV("Start tcp time(%lld)", end-start);
    return -1;
}

static cdx_int32 __CdxTcpStreamConnect(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;
    cdx_int32 result;
    
    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);
    
    pthread_mutex_lock(&impl->stateLock);
    if(impl->forceStopFlag)
    {
        pthread_mutex_unlock(&impl->stateLock);
        return -1;
    }
    CdxAtomicSet(&impl->state, TCP_STREAM_CONNECTING);
    CdxAtomicInc(&impl->ref);
    pthread_mutex_unlock(&impl->stateLock);

    
    result = StartTcpStreamConnect(impl);
    if (result < 0)
    {
        CDX_LOGE("StartTcpStreamConnect failed!");
        pthread_mutex_lock(&impl->stateLock);
        impl->ioState = CDX_IO_STATE_ERROR;
        pthread_mutex_unlock(&impl->stateLock);
    }
    else
    {
    	pthread_mutex_lock(&impl->stateLock);
        impl->ioState = CDX_IO_STATE_OK;
        pthread_mutex_unlock(&impl->stateLock);
    }
        
    pthread_mutex_lock(&impl->stateLock);
    CdxAtomicSet(&impl->state, TCP_STREAM_IDLE);
    CdxTcpStreamDecRef(&impl->base);
    pthread_mutex_unlock(&impl->stateLock);
    return (impl->ioState == CDX_IO_STATE_ERROR) ? -1 : 0;
}

static struct CdxStreamOpsS CdxTcpStreamOps = {
    .connect    = __CdxTcpStreamConnect,
    .read       = __CdxTcpStreamRead,
    .close      = __CdxTcpStreamClose,
    .getIOState = __CdxTcpStreamGetIOState,
//    .forceStop  = __CdxTcpStreamForceStop,
    .write      = __CdxTcpStreamWrite,
    .control    = __CdxTcpStreamControl
};

static CdxStreamT *__CdxTcpStreamCreate(CdxDataSourceT *source)
{
    CdxTcpStreamImplT *impl = NULL;
    cdx_int32 sockRecvLen;
    cdx_int32 fd = -1;
    
    impl = (CdxTcpStreamImplT *)malloc(sizeof(CdxTcpStreamImplT));
    if(NULL == impl)
    {
        CDX_LOGE("malloc failed");
        return NULL;
    }
    memset(impl, 0x00, sizeof(CdxTcpStreamImplT));
    impl->base.ops = &CdxTcpStreamOps;
    impl->ioState = CDX_IO_STATE_INVALID;

    fd = CdxAsynSocket(0/*SOCKRECVBUF_LEN*/, &sockRecvLen);
    if(fd < 0)
    {
        impl->ioState = CDX_IO_STATE_ERROR;
        CDX_LOGE("create socket failed.");
        goto err_out;
    }
    impl->sockFd = fd;
    impl->sockRecvBufLen = sockRecvLen;
    impl->port = *(cdx_int32 *)((CdxHttpSendBufferT *)source->extraData)->size;
    impl->hostname = (char *)((CdxHttpSendBufferT *)source->extraData)->buf;
    //CDX_LOGV("port (%d), hostname(%s)", impl->port, impl->hostname);    
    CdxAtomicSet(&impl->ref, 1);
    pthread_mutex_init(&impl->stateLock, NULL);

    pthread_mutex_init(&impl->dnsMutex, NULL);
    pthread_cond_init(&impl->dnsCond, NULL);
    impl->dnsRet = SDS_PENDING;
	CdxAtomicSet(&impl->state, TCP_STREAM_IDLE);
    
    return &impl->base;
    
err_out:
    if(impl)
    {
        free(impl);
        impl = NULL;
    }
    return NULL;
}

CdxStreamCreatorT tcpStreamCtor = {
    .create = __CdxTcpStreamCreate
};

