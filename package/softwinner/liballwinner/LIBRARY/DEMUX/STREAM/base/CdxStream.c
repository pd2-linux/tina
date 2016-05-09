#include <CdxTypes.h>
#include <CdxLog.h>
#include <CdxMemory.h>

#include <CdxStream.h>
#include "cdx_config.h"
#include <CdxList.h>

/*
#define STREAM_DECLARE(scheme) \
    extern CdxStreamCreatorT cdx_##scheme##_stream_ctor

#define STREAM_REGISTER(scheme) \
    {#scheme, &cdx_##scheme##_stream_ctor}
*/

struct CdxStreamListS
{
    CdxListT list;
    int size;
};

struct CdxStreamListS streamList;

extern CdxStreamCreatorT fileStreamCtor;

#if (CONFIG_HAVE_LIVE555 == OPTION_HAVE_LIVE555)
extern CdxStreamCreatorT rtspStreamCtor;
#endif

extern CdxStreamCreatorT httpStreamCtor;
extern CdxStreamCreatorT tcpStreamCtor;
//extern CdxStreamCreatorT rtmpStreamCtor;
//extern CdxStreamCreatorT mmsStreamCtor;
extern CdxStreamCreatorT udpStreamCtor;
//extern CdxStreamCreatorT rtpStreamCreator;
//extern CdxStreamCreatorT customerStreamCtor;
//extern CdxStreamCreatorT sslStreamCtor;
//extern CdxStreamCreatorT aesStreamCtor;
//extern CdxStreamCreatorT bdmvStreamCtor;
//extern CdxStreamCreatorT widevineStreamCtor;
//extern CdxStreamCreatorT videoResizeStreamCtor;


struct CdxStreamNodeS
{
    CdxListNodeT node;
    const cdx_char *scheme;
    CdxStreamCreatorT *creator;
};

int AwStreamRegister(CdxStreamCreatorT *creator, cdx_char *type)
{
    struct CdxStreamNodeS *streamNode;

    streamNode = malloc(sizeof(*streamNode));
    streamNode->creator = creator;
    streamNode->scheme = type;
    
    CdxListAddTail(&streamNode->node, &streamList.list);
    streamList.size++;
    return 0;
}

static cdx_void AwStreamInit(cdx_void) __attribute__((constructor));
cdx_void AwStreamInit(cdx_void)
{
    CDX_LOGD("aw stream init...");
    CdxListInit(&streamList.list);
    streamList.size = 0;
    
    AwStreamRegister(&fileStreamCtor,"fd");
    AwStreamRegister(&fileStreamCtor,"file");
#if (CONFIG_HAVE_LIVE555 == OPTION_HAVE_LIVE555)
    AwStreamRegister(&rtspStreamCtor,"rtsp");
#endif
    AwStreamRegister(&httpStreamCtor,"http");
//    AwStreamRegister(&httpStreamCtor,"https");
    AwStreamRegister(&tcpStreamCtor,"tcp");
//    AwStreamRegister(&rtmpStreamCtor,"rtmp");
//    AwStreamRegister(&mmsStreamCtor,"mms");
//    AwStreamRegister(&mmsStreamCtor,"mmsh");
//    AwStreamRegister(&mmsStreamCtor,"mmst");
//    AwStreamRegister(&mmsStreamCtor,"mmshttp");
    AwStreamRegister(&udpStreamCtor,"udp");
//    AwStreamRegister(&customerStreamCtor,"customer");
//    AwStreamRegister(&sslStreamCtor,"ssl");
//    AwStreamRegister(&aesStreamCtor,"aes");
//    AwStreamRegister(&bdmvStreamCtor,"bdmv");
#if CONFIG_OS == OPTION_OS_ANDROID
//    AwStreamRegister(&widevineStreamCtor,"widevine");
#endif
//    AwStreamRegister(&videoResizeStreamCtor,"videoResize");

	CDX_LOGD("stream list size:%d",streamList.size);
    return ;
}

CdxStreamT *CdxStreamCreate(CdxDataSourceT *source)
{
    cdx_char scheme[16] = {0};
    cdx_char *colon;
    CdxStreamCreatorT *ctor = NULL;
    struct CdxStreamNodeS *streamNode;

    colon = strchr(source->uri, ':');
    CDX_CHECK(colon && (colon - source->uri < 16));
    memcpy(scheme, source->uri, colon - source->uri);
    
	CdxListForEachEntry(streamNode, &streamList.list, node)
	{
		CDX_CHECK(streamNode->creator);
        if (strcasecmp(streamNode->scheme, scheme) == 0)
        {
            ctor = streamNode->creator;
            break;
        }
	}

    if (NULL == ctor)
    {
        CDX_LOGE("unsupport stream. scheme(%s)", scheme);
        return NULL;
    }

    CDX_CHECK(ctor->create);
    CdxStreamT *stream = ctor->create(source);
    if (!stream)
    {
        CDX_LOGE("open stream fail, uri(%s)", source->uri);
        return NULL;
    }
    
    return stream;
}

int CdxStreamOpen(CdxDataSourceT *source, pthread_mutex_t *mutex, cdx_bool *exit, CdxStreamT **stream, ContorlTask *streamTasks)
{
	if(mutex)
		pthread_mutex_lock(mutex);
	
    if(exit && *exit)
    {
	CDX_LOGW("open stream user cancel.");
        if(mutex) pthread_mutex_unlock(mutex);
	return -1;
    }
    *stream = CdxStreamCreate(source);
	if(mutex) pthread_mutex_unlock(mutex);
    if (!*stream)
    {
	CDX_LOGW("open stream failure.");
	return -1;
    }
	int ret;
	while(streamTasks)
	{
		ret = CdxStreamControl(*stream, streamTasks->cmd, streamTasks->param);
		if(ret < 0)
		{
			CDX_LOGE("CdxStreamControl fail, cmd=%d", streamTasks->cmd);
			return ret;
		}
		streamTasks = streamTasks->next;
	}
    return CdxStreamConnect(*stream);
}
