#define LOG_TAG "recoderdemo"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#include <config.h>
#include <log.h>

#include <CdxQueue.h>
#include <AwPool.h>
#include <CdxBinary.h>
#include <CdxMuxer.h>

#include "memoryAdapter.h"
#include "awencoder.h"

#define SAVE_VIDEO_FRAME (0)

#if CONFIG_CHIP == OPTION_CHIP_1639
#define PHY_OFFSET 0x20000000
#else
#define PHY_OFFSET 0x40000000
#endif


static const int STATUS_IDEL   = 0;

typedef struct DemoRecoderContext
{
	AwEncoder*       mAwEncoder;
	int callbackData;             
	pthread_mutex_t mMutex;

    VideoEncodeConfig videoConfig;
    AudioEncodeConfig audioConfig;


	CdxMuxerT* pMuxer;
	int muxType;
	char pUrl[1024];
	CdxStreamT* pStream;
	char*       pOutUrl;

    unsigned char* extractDataBuff;
    unsigned int extractDataLength;
	
    pthread_t muxerThreadId ;
    AwPoolT *pool;
    CdxQueueT *dataQueue;
    int exitFlag ;
	unsigned char* pAddrPhyY;
	unsigned char* pAddrPhyC;
	int     bUsed;

	FILE* fpSaveVideoFrame;
}DemoRecoderContext;

//* a notify callback for AwEncorder.
void NotifyCallbackForAwEncorder(void* pUserData, int msg, void* param)
{
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)pUserData;

    switch(msg)
    {
		case AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_BUFFER:
		{
			int id = *((int*)param);
			if(id == 0)
			{
				pDemoRecoder->bUsed = 0;
				printf("---- pDemoRecoder->bUsed: %d , %p, pDemoRecoder: %p\n", pDemoRecoder->bUsed, &pDemoRecoder->bUsed, pDemoRecoder);
			}
			break;
		}
        default:
        {
            printf("warning: unknown callback from AwRecorder.\n");
            break;
        }
    }
    
    return ;
}

int onVideoDataEnc(void *app,CdxMuxerPacketT *buff)
{
    CdxMuxerPacketT *packet = NULL;
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)app;
    if (!buff)
        return 0;

    packet = (CdxMuxerPacketT*)malloc(sizeof(CdxMuxerPacketT));
    packet->buflen = buff->buflen;
    packet->length = buff->length;
    packet->buf = malloc(buff->buflen);
    memcpy(packet->buf, buff->buf, packet->buflen);
    packet->pts = buff->pts;
    packet->type = buff->type;
    packet->streamIndex  = buff->streamIndex;
    packet->duration = buff->duration;

#if SAVE_VIDEO_FRAME   
	if(pDemoRecoder->fpSaveVideoFrame)
	{
		fwrite(packet->buf, 1, packet->buflen, pDemoRecoder->fpSaveVideoFrame);
	}
#endif

    CdxQueuePush(pDemoRecoder->dataQueue,packet);
    return 0;
}
int onAudioDataEnc(void *app,CdxMuxerPacketT *buff)
{
    CdxMuxerPacketT *packet = NULL;
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)app;
    if (!buff)
        return 0;
    packet = (CdxMuxerPacketT*)malloc(sizeof(CdxMuxerPacketT));
    packet->buflen = buff->buflen;
    packet->length = buff->length;
    packet->buf = malloc(buff->buflen);
    memcpy(packet->buf, buff->buf, packet->buflen);
    packet->pts = buff->pts;
    packet->type = buff->type;
    packet->streamIndex = buff->streamIndex;

    CdxQueuePush(pDemoRecoder->dataQueue,packet);

    return 0;
}

void* MuxerThread(void *param)
{
    int ret = 0;
    int i =0;
    CdxMuxerPacketT *mPacket = NULL;
	DemoRecoderContext *p = (DemoRecoderContext*)param;

    logd("MuxerThread");

    
	if(p->pUrl)
	{
		CdxDataSourceT dataSource;
		dataSource.uri = strdup(p->pUrl);

		p->pStream = CdxStreamCreate(&dataSource);
		if(p->pStream == NULL)
		{
			loge("CdxStreamCreate failed");
			return 0;
		}
		
		ret = CdxStreamConnect(p->pStream);
		if(ret < 0)
		{
			loge("CdxStreamConnect failed(%s)", p->pUrl);
			return 0;
		}
		free(dataSource.uri);

		p->pMuxer = CdxMuxerCreate(p->muxType, p->pStream);
		if(p->pMuxer == NULL)
		{
			loge("CdxMuxerCreate failed");
			return 0;
		}
		logd("MuxerThread init ok");
	}

	CdxMuxerMediaInfoT mediainfo;

	switch (p->audioConfig.nType)
	{
		case AUDIO_ENCODE_PCM_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_PCM_TYPE;
			break;
		case AUDIO_ENCODE_AAC_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_AAC_TYPE;
			break;
		case AUDIO_ENCODE_MP3_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_MP3_TYPE;
			break;
		case AUDIO_ENCODE_LPCM_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_LPCM_TYPE;
			break;
		default:
			loge("unlown audio type(%d)", p->audioConfig.nType);
			break;
	}

    mediainfo.audioNum = 1;
    mediainfo.videoNum = 1;
	
	mediainfo.audio.nAvgBitrate = p->audioConfig.nBitrate;
	mediainfo.audio.nBitsPerSample = p->audioConfig.nSamplerBits;
	mediainfo.audio.nChannelNum = p->audioConfig.nOutChan;
	mediainfo.audio.nMaxBitRate = p->audioConfig.nBitrate;
	mediainfo.audio.nSampleRate = p->audioConfig.nOutSamplerate;
	mediainfo.audio.nSampleCntPerFrame = 1024; // aac

	if(p->videoConfig.nType == VIDEO_ENCODE_H264)
		mediainfo.video.eCodeType = VENC_CODEC_H264;
	else if(p->videoConfig.nType == VIDEO_ENCODE_JPEG)
		mediainfo.video.eCodeType = VENC_CODEC_JPEG;
	else
	{
		loge("cannot suppot this video type");
		return 0;
	}

	mediainfo.video.nWidth    = p->videoConfig.nOutWidth;
	mediainfo.video.nHeight   = p->videoConfig.nOutHeight;
	mediainfo.video.nFrameRate = p->videoConfig.nFrameRate;

	logd("******************* mux mediainfo *****************************");
	logd("videoNum                   : %d", mediainfo.videoNum);
	logd("audioNum                   : %d", mediainfo.audioNum);
	logd("videoTYpe                  : %d", mediainfo.video.eCodeType);
	logd("framerate                  : %d", mediainfo.video.nFrameRate);
	logd("width                      : %d", mediainfo.video.nWidth);
	logd("height                     : %d", mediainfo.video.nHeight);
	logd("**************************************************************");

	if(p->pMuxer)
	{
		CdxMuxerSetMediaInfo(p->pMuxer, &mediainfo);	
	}

    logd("extractDataLength %d",p->extractDataLength);
    if(p->extractDataLength > 0 && p->pMuxer)
    {
        logd("demo WriteExtraData");
        if(p->pMuxer)
            CdxMuxerWriteExtraData(p->pMuxer, p->extractDataBuff, p->extractDataLength, 0);
    }
    
	if(p->pMuxer)
	{
	    logd("write head");
		CdxMuxerWriteHeader(p->pMuxer);
	}

    while (i <50)
    {
        while (!CdxQueueEmpty(p->dataQueue))
        {
            mPacket = CdxQueuePop(p->dataQueue);
            i++;
            if(p->pMuxer)
            {
                logd("write packet");
                if(CdxMuxerWritePacket(p->pMuxer, mPacket) < 0)
                {
                    loge("+++++++ CdxMuxerWritePacket failed");
                }
            }
            
            free(mPacket->buf);
            free(mPacket);
            
        }
        usleep(1*1000);

    }

    if(p->pMuxer)
    {
        logd("write trailer");
        CdxMuxerWriteTrailer(p->pMuxer);
    }

    logd("CdxMuxerClose");
    if(p->pMuxer)
		CdxMuxerClose(p->pMuxer);
    p->pMuxer = NULL;


	p->exitFlag  = 1;

    return 0;
}


//* the main method.
int main(int argc, char *argv[]) 
{
    DemoRecoderContext demoRecoder;
    int  bQuit;
    
    EncDataCallBackOps mEncDataCallBackOps;
    CdxMuxerPacketT *mPacket = NULL;

    mEncDataCallBackOps.onAudioDataEnc = onAudioDataEnc;
    mEncDataCallBackOps.onVideoDataEnc = onVideoDataEnc;

    printf("\n");
    printf("******************************************************************************************\n");
    printf("* This program implements a simple recoder.\n");
    printf("* Inplemented by Allwinner ALD-AL3 department.\n");
    printf("******************************************************************************************\n");

    if(argc < 3)
    {
    	printf("run failed \n");
    	printf("./recoderdemo argv[1] argv[2] \n");
    	printf(" argv[1]: video yuv data file \n");
    	printf(" argv[2]: audio pcm file \n");
    	return -1;
    }
    
    //* create a demoRecoder.
    memset(&demoRecoder, 0, sizeof(DemoRecoderContext));

    demoRecoder.pool = AwPoolCreate(NULL);
    demoRecoder.dataQueue = CdxQueueCreate(demoRecoder.pool);

    demoRecoder.fpSaveVideoFrame = fopen("/mnt/UDISK/video.dat", "wb");
    if(demoRecoder.fpSaveVideoFrame == NULL)
    {
    	printf("open file /mnt/UDISK/video.dat failed, errno(%d)\n", errno);
    }

	//VideoEncodeConfig videoConfig;
	memset(&demoRecoder.videoConfig, 0x00, sizeof(VideoEncodeConfig));
	demoRecoder.videoConfig.nType = VIDEO_ENCODE_JPEG;
	demoRecoder.videoConfig.nFrameRate = 30;
	demoRecoder.videoConfig.nOutHeight = 720;
	demoRecoder.videoConfig.nOutWidth = 1280;
	demoRecoder.videoConfig.nSrcHeight = 720;
	demoRecoder.videoConfig.nSrcWidth = 1280;
	demoRecoder.videoConfig.nBitRate = 3*1000*1000;
	demoRecoder.videoConfig.bUsePhyBuf  = 1;

	//AudioEncodeConfig audioConfig;	
	demoRecoder.audioConfig.nType = AUDIO_ENCODE_PCM_TYPE;
	demoRecoder.audioConfig.nInChan = 2;
	demoRecoder.audioConfig.nInSamplerate = 44100;
	demoRecoder.audioConfig.nOutChan = 2;
	demoRecoder.audioConfig.nOutSamplerate = 44100;
	demoRecoder.audioConfig.nSamplerBits = 16;

    demoRecoder.muxType = CDX_MUXER_MOV;
    if(demoRecoder.muxType == CDX_MUXER_TS && demoRecoder.audioConfig.nType == AUDIO_ENCODE_PCM_TYPE)
    {
    	demoRecoder.audioConfig.nFrameStyle = 2;
    }

    if((demoRecoder.muxType == CDX_MUXER_TS) 
    	&& demoRecoder.audioConfig.nType == AUDIO_ENCODE_AAC_TYPE)
    {
    	demoRecoder.audioConfig.nFrameStyle = 1;
    }
    
    pthread_mutex_init(&demoRecoder.mMutex, NULL);

	demoRecoder.mAwEncoder = AwEncoderCreate(&demoRecoder);
    if(demoRecoder.mAwEncoder == NULL)
    {
        printf("can not create AwRecorder, quit.\n");
        exit(-1);
    }
    
    //* set callback to recoder.
    AwEncoderSetNotifyCallback(demoRecoder.mAwEncoder,NotifyCallbackForAwEncorder,&(demoRecoder));

    AwEncoderInit(demoRecoder.mAwEncoder, &demoRecoder.videoConfig, &demoRecoder.audioConfig,&mEncDataCallBackOps); 
    //AwEncoderInit(demoRecoder.mAwEncoder, &demoRecoder.videoConfig, NULL,&mEncDataCallBackOps);


    sprintf(demoRecoder.pUrl,  "write:///mnt/UDISK/save.mp4");

    AwEncoderStart(demoRecoder.mAwEncoder);

    AwEncoderGetExtradata(demoRecoder.mAwEncoder,&demoRecoder.extractDataBuff,&demoRecoder.extractDataLength);
    if(demoRecoder.fpSaveVideoFrame)
    {
    	fwrite(demoRecoder.extractDataBuff, 1, demoRecoder.extractDataLength, demoRecoder.fpSaveVideoFrame);
    }

    pthread_create(&demoRecoder.muxerThreadId, NULL, MuxerThread, &demoRecoder);
    
    //* read, parse and process command from user.
    bQuit = 0;

    FILE* inputYUV = fopen(argv[1], "rb");
    if(inputYUV == NULL)
    {
    	printf("open yuv file failed");
    	return -1;
    }

    FILE* inputPCM = fopen(argv[2], "rb");
    if(inputPCM == NULL)
    {
    	printf("open yuv file failed");
    	return -1;
    }

    AudioInputBuffer audioInputBuffer;
    memset(&audioInputBuffer, 0x00, sizeof(AudioInputBuffer));
    audioInputBuffer.nLen = 10240; //176400;
    audioInputBuffer.pData = (char*)malloc(audioInputBuffer.nLen);

    VideoInputBuffer videoInputBuffer;
    int sizeY = demoRecoder.videoConfig.nSrcHeight* demoRecoder.videoConfig.nSrcWidth;
    if(demoRecoder.videoConfig.bUsePhyBuf)
    {
    	demoRecoder.pAddrPhyY = MemAdapterPalloc(sizeY);
		demoRecoder.pAddrPhyC = MemAdapterPalloc(sizeY/2);
		printf("==== palloc demoRecoder.pAddrPhyY: %p\n", demoRecoder.pAddrPhyY);
/*
		videoInputBuffer.nID = 0;
		fread(demoRecoder.pAddrPhyY, 1, sizeY,  inputYUV);
		fread(demoRecoder.pAddrPhyC, 1, sizeY/2, inputYUV);
		MemAdapterFlushCache(demoRecoder.pAddrPhyY, sizeY);
		MemAdapterFlushCache(demoRecoder.pAddrPhyC, sizeY/2);

		videoInputBuffer.nID = 0;
		videoInputBuffer.pAddrPhyY = MemAdapterGetPhysicAddress(demoRecoder.pAddrPhyY)+PHY_OFFSET;
		videoInputBuffer.pAddrPhyC = MemAdapterGetPhysicAddress(demoRecoder.pAddrPhyC)+PHY_OFFSET;
		*/
    }
    else
    {
	    memset(&videoInputBuffer, 0x00, sizeof(VideoInputBuffer));
	    videoInputBuffer.nLen = demoRecoder.videoConfig.nSrcHeight* demoRecoder.videoConfig.nSrcWidth *3/2;
	    videoInputBuffer.pData = (unsigned char*)malloc(videoInputBuffer.nLen);
    }
    
    unsigned int size1, size2;
    int videoEos = 0;
    int audioEos = 0;
    long long audioPts = 0; 
    long long videoPts = 0;
	
    while(!bQuit)
    {
    	int ret = -1;
    	int num = 0;
    	int used = 0;
    	while(num<50)
    	{
    		ret = -1;
    		if(!videoEos)
    		{
				if(demoRecoder.videoConfig.bUsePhyBuf)
				{
					while(1)
					{
						if(demoRecoder.bUsed == 0)
						{
							break;
						}
						//printf("==== wait buf return, demoRecoder.bUsed: %d \n", demoRecoder.bUsed);
						usleep(10000);
					}
					
					videoInputBuffer.nID = 0;
					fread(demoRecoder.pAddrPhyY, 1, sizeY,  inputYUV);
					fread(demoRecoder.pAddrPhyC, 1, sizeY/2, inputYUV);
					MemAdapterFlushCache(demoRecoder.pAddrPhyY, sizeY);
					MemAdapterFlushCache(demoRecoder.pAddrPhyC, sizeY/2);

					videoInputBuffer.nID = 0;
					videoInputBuffer.pAddrPhyY = MemAdapterGetPhysicAddress(demoRecoder.pAddrPhyY)+PHY_OFFSET;
					videoInputBuffer.pAddrPhyC = MemAdapterGetPhysicAddress(demoRecoder.pAddrPhyC)+PHY_OFFSET;
					
				}
				else
				{
					size1 = fread(videoInputBuffer.pData, 1, videoInputBuffer.nLen, inputYUV);
			    	if(size1 < videoInputBuffer.nLen)
			    	{
			    		logd("read error");
			    		videoEos = 1;
			    	}
				}
	    	
		    	while(ret < 0)
				{
				    videoInputBuffer.nPts = videoPts;
				    demoRecoder.bUsed = 1;
				    //printf("==== writeYUV used: %d", demoRecoder.bUsed);
					ret = AwEncoderWriteYUVdata(demoRecoder.mAwEncoder,&videoInputBuffer);
				
					usleep(10*1000);
				}
				videoPts += 30;
	    	}

			ret = -1;
	    	if(!audioEos)
	    	{
				size2 = fread(audioInputBuffer.pData, 1, audioInputBuffer.nLen, inputPCM);
				if(size2 < audioInputBuffer.nLen)
				{
					logd("read error");
					audioEos = 1;
				}

				{
				    audioInputBuffer.nPts = audioPts;
					ret = AwEncoderWritePCMdata(demoRecoder.mAwEncoder,&audioInputBuffer);
					if(ret < 0)
					{
						audioEos = 1;
					}
				}
				audioPts += 1000;
			}

			printf("num: %d \n", num);
			num ++;
		}
        logd("read data finish!");
		break;

	}
	
    while (demoRecoder.exitFlag == 0)
    {
        logd("wait MuxerThread finish!");
        usleep(1000*1000);
    }

	printf("destroy AwRecorder.\n");
	
    while (!CdxQueueEmpty(demoRecoder.dataQueue))
    {
        logd("free a packet");
        mPacket = CdxQueuePop(demoRecoder.dataQueue);
        free(mPacket->buf);
        free(mPacket);
    }
    CdxQueueDestroy(demoRecoder.dataQueue);
    AwPoolDestroy(demoRecoder.pool);
    
	printf("==== freee demoRecoder.pAddrPhyY: %p\n", demoRecoder.pAddrPhyY);
	if(demoRecoder.pAddrPhyY)
	{
		MemAdapterPfree(demoRecoder.pAddrPhyY);
	}
	printf("==== freee demoRecoder.pAddrPhyY  end\n");

    if(demoRecoder.pAddrPhyC)
	{
		MemAdapterPfree(demoRecoder.pAddrPhyC);
	}
    
	if(demoRecoder.mAwEncoder != NULL)
	{
		AwEncoderStop(demoRecoder.mAwEncoder);
	    AwEncoderDestory(demoRecoder.mAwEncoder);
	    demoRecoder.mAwEncoder = NULL;
	}
	
    pthread_mutex_destroy(&demoRecoder.mMutex);

    if(demoRecoder.fpSaveVideoFrame)
    	fclose(demoRecoder.fpSaveVideoFrame);

    fclose(inputYUV);
    fclose(inputPCM);
    
    printf("\n");
    printf("******************************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("******************************************************************************************\n");
    printf("\n");
	
	return 0;
}
