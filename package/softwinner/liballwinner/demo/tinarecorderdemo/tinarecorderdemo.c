
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#include <CdxQueue.h>
#include <AwPool.h>
#include <CdxBinary.h>
#include <CdxMuxer.h>

#include "memoryAdapter.h"
#include "awencoder.h"

#include "RecorderWriter.h"

#define SAVE_VIDEO_FRAME (0)
#define SAVE_AUDIO_FRAME (0)

static const int STATUS_IDEL   = 0;

FILE* inputPCM = NULL;
FILE* inputYUV = NULL;

char pcm_path[128] = {0};
char yuv_path[128] = {0};

int videoEos = 0;
int audioEos = 0;

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
	CdxWriterT* pStream;
	char*       pOutUrl;

    unsigned char* extractDataBuff;
    unsigned int extractDataLength;
	
    pthread_t muxerThreadId ;
    pthread_t audioDataThreadId ;
    pthread_t videoDataThreadId ;
    AwPoolT *pool;
    CdxQueueT *dataQueue;
    int exitFlag ;
	unsigned char* pAddrPhyY;
	unsigned char* pAddrPhyC;
	int     bUsed;

	FILE* fpSaveVideoFrame;
	FILE* fpSaveAudioFrame;
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
				//printf("---- pDemoRecoder->bUsed: %d , %p, pDemoRecoder: %p\n", pDemoRecoder->bUsed, &pDemoRecoder->bUsed, pDemoRecoder);
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
#if SAVE_AUDIO_FRAME
		if(pDemoRecoder->fpSaveAudioFrame)
		{
			fwrite(packet->buf, 1, packet->buflen, pDemoRecoder->fpSaveAudioFrame);
		}
#endif

    CdxQueuePush(pDemoRecoder->dataQueue,packet);

    return 0;
}

void* MuxerThread(void *param)
{
    //int ret = 0;
    int i =0;
    CdxMuxerPacketT *mPacket = NULL;
	DemoRecoderContext *p = (DemoRecoderContext*)param;
	RecoderWriterT *rw = NULL;
#if FS_WRITER
	CdxFsCacheMemInfo fs_cache_mem;
#endif
    printf("MuxerThread begin\n");
    
	if(p->pUrl)
	{
        if ((p->pStream = CdxWriterCreat()) == NULL)
        {
            printf("CdxWriterCreat() failed\n");
            return 0;
        }
        rw = (RecoderWriterT*)p->pStream;
        rw->file_mode = FD_FILE_MODE;
        strcpy(rw->file_path, p->pUrl);
        RWOpen(p->pStream);
		p->pMuxer = CdxMuxerCreate(p->muxType, p->pStream);
		if(p->pMuxer == NULL)
		{
			printf("CdxMuxerCreate failed\n");
			return 0;
		}
		printf("MuxerThread init ok\n");
	}

	CdxMuxerMediaInfoT mediainfo;
    memset(&mediainfo, 0, sizeof(CdxMuxerMediaInfoT));
	if(inputPCM != NULL){
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
				printf("unlown audio type(%d)\n", p->audioConfig.nType);
				break;
		}
	    mediainfo.audioNum = 1;
		mediainfo.audio.nAvgBitrate = p->audioConfig.nBitrate;
		mediainfo.audio.nBitsPerSample = p->audioConfig.nSamplerBits;
		mediainfo.audio.nChannelNum = p->audioConfig.nOutChan;
		mediainfo.audio.nMaxBitRate = p->audioConfig.nBitrate;
		mediainfo.audio.nSampleRate = p->audioConfig.nOutSamplerate;
		mediainfo.audio.nSampleCntPerFrame = 1024; // 固定的,一帧数据的采样点数目
	}
	
	if((inputYUV!=NULL)&&(p->muxType == CDX_MUXER_MOV || p->muxType == CDX_MUXER_TS)){
		mediainfo.videoNum = 1;
		if(p->videoConfig.nType == VIDEO_ENCODE_H264)
			mediainfo.video.eCodeType = VENC_CODEC_H264;
		else if(p->videoConfig.nType == VIDEO_ENCODE_JPEG)
			mediainfo.video.eCodeType = VENC_CODEC_JPEG;
		else
		{
			printf("cannot suppot this video type\n");
			return 0;
		}
		mediainfo.video.nWidth    = p->videoConfig.nOutWidth;
		mediainfo.video.nHeight   = p->videoConfig.nOutHeight;
		mediainfo.video.nFrameRate = p->videoConfig.nFrameRate;
	}

	printf("******************* mux mediainfo *****************************\n");
	printf("videoNum                   : %d\n", mediainfo.videoNum);
	printf("videoTYpe                  : %d\n", mediainfo.video.eCodeType);
	printf("framerate                  : %d\n", mediainfo.video.nFrameRate);
	printf("width                      : %d\n", mediainfo.video.nWidth);
	printf("height                     : %d\n", mediainfo.video.nHeight);
	printf("audioNum                   : %d\n", mediainfo.audioNum);
	printf("audioFormat                : %d\n", mediainfo.audio.eCodecFormat);
	printf("audioChannelNum            : %d\n", mediainfo.audio.nChannelNum);
	printf("audioSmpleRate             : %d\n", mediainfo.audio.nSampleRate);
	printf("audioBitsPerSample         : %d\n", mediainfo.audio.nBitsPerSample);
	printf("**************************************************************\n");

	if(p->pMuxer)
	{
		CdxMuxerSetMediaInfo(p->pMuxer, &mediainfo);	
#if FS_WRITER
        memset(&fs_cache_mem, 0, sizeof(CdxFsCacheMemInfo));

        fs_cache_mem.m_cache_size = 512 * 1024;
        fs_cache_mem.mp_cache = (cdx_int8*)malloc(fs_cache_mem.m_cache_size);
        if (fs_cache_mem.mp_cache == NULL)
        {
            printf("fs_cache_mem.mp_cache malloc failed\n");
            return NULL;
        }
        CdxMuxerControl(p->pMuxer, SET_CACHE_MEM, &fs_cache_mem);
        int fs_mode = FSWRITEMODE_CACHETHREAD;

        /*
	        fs_cache_size = 1024 * 1024;
	        CdxMuxerControl(p->pMuxer, SET_FS_SIMPLE_CACHE_SIZE, &fs_cache_size);
	        fs_mode = FSWRITEMODE_SIMPLECACHE;
	        */
        //fs_mode = FSWRITEMODE_DIRECT;
        CdxMuxerControl(p->pMuxer, SET_FS_WRITE_MODE, &fs_mode);
#endif
    }

    printf("extractDataLength %u\n",p->extractDataLength);
    if(p->extractDataLength > 0 && p->pMuxer)
    {
        printf("demo WriteExtraData\n");
        if(p->pMuxer)
            CdxMuxerWriteExtraData(p->pMuxer, p->extractDataBuff, p->extractDataLength, 0);
    }
    
    if(p->pMuxer)
	{
	    printf("write head\n");
		CdxMuxerWriteHeader(p->pMuxer);
	}

    while ((audioEos==0) || (videoEos==0))
    {
        while (!CdxQueueEmpty(p->dataQueue))
        {
            mPacket = CdxQueuePop(p->dataQueue);
            i++;
            if(p->pMuxer)
            {
                if(CdxMuxerWritePacket(p->pMuxer, mPacket) < 0)
                {
                    printf("+++++++ CdxMuxerWritePacket failed\n");
                }
            }
            
            free(mPacket->buf);
            free(mPacket);
            
        }
        usleep(1*1000);

    }

    if(p->pMuxer)
    {
        printf("write trailer\n");
        CdxMuxerWriteTrailer(p->pMuxer);
    }

    printf("CdxMuxerClose\n");
    if(p->pMuxer)
    {
		CdxMuxerClose(p->pMuxer);
        p->pMuxer = NULL;
    }
    if(p->pStream)
    {
        RWClose(p->pStream);
        CdxWriterDestroy(p->pStream);
        p->pStream = NULL;
        rw = NULL;
    }

#if FS_WRITER
    if(fs_cache_mem.mp_cache)
    {
        free(fs_cache_mem.mp_cache);
        fs_cache_mem.mp_cache = NULL;
    }
#endif
    printf("MuxerThread has finish!\n");

    return 0;
}


void* AudioInputThread(void *param)
{
    int ret = 0;
    //int i =0;
	DemoRecoderContext *p = (DemoRecoderContext*)param;

    printf("AudioInputThread\n");
    int num = 0;
    int size2 = 0;
    int64_t audioPts = 0;
    
    AudioInputBuffer audioInputBuffer;
    memset(&audioInputBuffer, 0x00, sizeof(AudioInputBuffer));
    audioInputBuffer.nLen = 1024*4; //176400;
    audioInputBuffer.pData = (char*)malloc(audioInputBuffer.nLen);

	while(num<10240)
	{
		ret = -1;
		if(!audioEos)
		{
		    if (inputPCM == NULL)
		    {
		        printf("before fread, inputPCM is NULL\n");
                break;
		    }
			size2 = fread(audioInputBuffer.pData, 1, audioInputBuffer.nLen, inputPCM);
			if(size2 < audioInputBuffer.nLen)
			{
				printf("audio has read to end,read from the begining again\n");
				fseek(inputPCM,0,SEEK_SET);
				continue;
				//printf("read error\n");
				//audioEos = 1;
			}

			while(ret < 0)
			{
				audioInputBuffer.nPts = audioPts;
				ret = AwEncoderWritePCMdata(p->mAwEncoder,&audioInputBuffer);
				//printf("=== WritePCMdata audioPts : %lld\n", audioPts);
				//printf("after write pcm ,sleep 10ms ,because the encoder maybe has no enough buf,ret = %d\n",ret);
				if(ret<0){
					printf("write pcm fail\n");
				}
				usleep(10*1000);
			}
			usleep(20*1000);
			audioPts += 23;
		}
		num ++;
	}
	printf("audio read data finish!\n");
	audioEos = 1;
	if (inputPCM)
    {
        fclose(inputPCM);
        inputPCM = NULL;
    }
    return 0;
}

void* VideoInputThread(void *param)
{
	DemoRecoderContext *p = (DemoRecoderContext*)param;

    printf("VideoInputThread\n");
    struct ScMemOpsS* memops = NULL;
    
    VideoInputBuffer videoInputBuffer;
    int sizeY = p->videoConfig.nSrcHeight* p->videoConfig.nSrcWidth;

    if(p->videoConfig.bUsePhyBuf)
    {
        memops = MemAdapterGetOpsS();
	    if(memops == NULL)
	    {
	        printf("memops is NULL\n");
		    return NULL;
	    }
	    CdcMemOpen(memops);
		p->pAddrPhyY = CdcMemPalloc(memops, sizeY);
		p->pAddrPhyC = CdcMemPalloc(memops, sizeY/2);
		printf("==== palloc demoRecoder.pAddrPhyY: %p\n", p->pAddrPhyY);
		printf("sizeY = %d\n",sizeY);
		printf("==== palloc demoRecoder.pAddrPhyC: %p\n", p->pAddrPhyC);
    }
    else
    {
	    memset(&videoInputBuffer, 0x00, sizeof(VideoInputBuffer));
	    videoInputBuffer.nLen = p->videoConfig.nSrcHeight* p->videoConfig.nSrcWidth *3/2;
	    videoInputBuffer.pData = (unsigned char*)malloc(videoInputBuffer.nLen);
    }
    
    long long videoPts = 0;
	
	int ret = -1;
	int num = 0;

	//while(num<721)//23s
	while(num<10000)
	{
		ret = -1;

		if(p->videoConfig.bUsePhyBuf)
		{
			while(1)
			{
				if(p->bUsed == 0)//if bUsed==0,means this buf has been returned by video decoder
				{
					break;
				}
				//printf("==== wait buf return, demoRecoder.bUsed: %d \n", demoRecoder.bUsed);
				usleep(10000);
			}
			
			videoInputBuffer.nID = 0;
			int llh_sizey = fread(p->pAddrPhyY, 1, sizeY,  inputYUV);
			int llh_sizec = fread(p->pAddrPhyC, 1, sizeY/2, inputYUV);
			//printf("llh_sizey = %d,llh_sizec = %d\n",llh_sizey,llh_sizec);
			if((llh_sizey<sizeY) || (llh_sizec<(sizeY/2))){
				//printf("phy:video has read to end,read from the begining again\n");
				fseek(inputYUV,0,SEEK_SET);
				continue;
			}
			CdcMemFlushCache(memops, p->pAddrPhyY, sizeY);
			CdcMemFlushCache(memops, p->pAddrPhyC, sizeY/2);
			
			videoInputBuffer.pAddrPhyY = CdcMemGetPhysicAddressCpu(memops, p->pAddrPhyY);
			videoInputBuffer.pAddrPhyC = CdcMemGetPhysicAddressCpu(memops, p->pAddrPhyC);
			//printf("==== palloc videoInputBuffer.pAddrPhyY: %p\n", videoInputBuffer.pAddrPhyY);
			//printf("==== palloc videoInputBuffer.pAddrPhyC: %p\n", videoInputBuffer.pAddrPhyC);
			
		}
		else
		{
			int size1;
			size1 = fread(videoInputBuffer.pData, 1, videoInputBuffer.nLen, inputYUV);
			if(size1 < videoInputBuffer.nLen)
			{
				//printf("video has read to end,read from the begining again\n");
				fseek(inputYUV,0,SEEK_SET);
				continue;
				//printf("read error\n");
				//videoEos = 1;
			}
		}
	
		while(ret < 0)
		{
		    videoInputBuffer.nPts = videoPts;
		    p->bUsed = 1;
		    //printf("==== writeYUV used: %d", demoRecoder.bUsed);
			ret = AwEncoderWriteYUVdata(p->mAwEncoder,&videoInputBuffer);
			if(ret<0){
				printf("write yuv data fail\n");
			}
			usleep(10*1000);
		}
		usleep(30*1000);
		videoPts += 33;
		num ++;
	}
	
    printf("video read data finish!\n");

	videoEos = 1;

	printf("==== freee demoRecoder.pAddrPhyY: %p\n", p->pAddrPhyY);
	if(p->pAddrPhyY)
	{
		CdcMemPfree(memops, p->pAddrPhyY);
	}
	printf("==== freee demoRecoder.pAddrPhyY  end\n");

    if(p->pAddrPhyC)
	{
		CdcMemPfree(memops, p->pAddrPhyC);
	}
    if (memops)
    {
	    CdcMemClose(memops);
	    memops = NULL;
    }

    if (inputYUV)
    {
        fclose(inputYUV);
        inputYUV = NULL;
    }
    return 0;
}

static void showHelp(void)
{
    printf("******************************************************************************************\n");
    printf("the command usage is like this: \n");
	printf("./tinarecorderdemo argv[1] argv[2] argv[3] argv[4] argv[5] argv[6]\n");
	printf(" argv[1]: video yuv data absolute path \n");
	printf(" argv[2]: audio pcm data absolute path \n");
	printf(" argv[3]: output absolute path \n");
	printf(" argv[4]: video encoded format ,support: jpeg/h264 \n");
	printf(" argv[5]: audio encoded format ,support: pcm/aac/mp3 \n");
	printf(" argv[6]: muxer format ,support: mp4/ts/aac/mp3 \n");
    printf("******************************************************************************************\n");
}

//* the main method.
int main(int argc, char *argv[]) 
{
    DemoRecoderContext demoRecoder;
    
    EncDataCallBackOps mEncDataCallBackOps;
    CdxMuxerPacketT *mPacket = NULL;

    mEncDataCallBackOps.onAudioDataEnc = onAudioDataEnc;
    mEncDataCallBackOps.onVideoDataEnc = onVideoDataEnc;

	if((argc == 2)&&(strcmp(argv[1],"--help")==0 || strcmp(argv[1],"-h")==0)){
		showHelp();
		return -1;
	}
    if(argc < 7)
    {
		printf("run failed , the argc is less than 7 \n");
		showHelp();
		return -1;
    }
	printf("input yuv path = %s\n",argv[1]);
    inputYUV = fopen(argv[1], "rb");
    printf("fopen inputYUV == %p\n", inputYUV);
    if(inputYUV == NULL)
    {
		printf("open yuv file failed, errno(%d),strerror = %s\n", errno,strerror(errno));
    }
	printf("input pcm path = %s\n",argv[2]);
    inputPCM = fopen(argv[2], "rb");
    printf("fopen inputPCM == %p\n", inputPCM);
    if(inputPCM == NULL)
    {
		printf("open pcm file failed, errno(%d),strerror = %s\n", errno,strerror(errno));
    }
    
    //* create a demoRecoder.
    memset(&demoRecoder, 0, sizeof(DemoRecoderContext));

    demoRecoder.pool = AwPoolCreate(NULL);
    demoRecoder.dataQueue = CdxQueueCreate(demoRecoder.pool);

#if SAVE_VIDEO_FRAME
    demoRecoder.fpSaveVideoFrame = fopen("/mnt/UDISK/video.dat", "wb");
    if(demoRecoder.fpSaveVideoFrame == NULL)
    {
		printf("open file /mnt/UDISK/video.dat failed, errno(%d),strerror = %s\n", errno,strerror(errno));
    }
#endif

#if SAVE_AUDIO_FRAME
	demoRecoder.fpSaveAudioFrame = fopen("/mnt/UDISK/audio.dat", "wb");
	if(demoRecoder.fpSaveAudioFrame == NULL)
	{
		printf("open file /mnt/UDISK/audio.dat failed, errno(%d),strerror = %s\n", errno,strerror(errno));
	}
#endif
	//muxer type
	if(strcmp(argv[6],"mp4")==0){
		demoRecoder.muxType       = CDX_MUXER_MOV;
	}else if(strcmp(argv[6],"ts")==0){
		demoRecoder.muxType       = CDX_MUXER_TS;
	}else if(strcmp(argv[6],"aac")==0){
		demoRecoder.muxType       = CDX_MUXER_AAC;
	}else if(strcmp(argv[6],"mp3")==0){
		demoRecoder.muxType       = CDX_MUXER_MP3;
	}else{
		printf("the input muxer type is not support,use the default:mp4\n");
		demoRecoder.muxType       = CDX_MUXER_MOV;
	}
	printf(" muxer type = %d\n",demoRecoder.muxType);

	//VideoEncodeConfig videoConfig;
	memset(&demoRecoder.videoConfig, 0x00, sizeof(VideoEncodeConfig));
	if(inputYUV != NULL && (demoRecoder.muxType == CDX_MUXER_MOV || demoRecoder.muxType == CDX_MUXER_TS)){
		if(strcmp(argv[4],"jpeg")==0){
			demoRecoder.videoConfig.nType       = VIDEO_ENCODE_JPEG;
		}else if(strcmp(argv[4],"h264")==0){
			demoRecoder.videoConfig.nType       = VIDEO_ENCODE_H264;
		}else{
			printf("the input video encode type is not support,use the default:h264\n");
			demoRecoder.videoConfig.nType       = VIDEO_ENCODE_H264;
		}
		printf("video encode type = %d\n",demoRecoder.videoConfig.nType);
		demoRecoder.videoConfig.nInputYuvFormat = VENC_PIXEL_YUV420P;
		demoRecoder.videoConfig.nFrameRate  = 30;
		demoRecoder.videoConfig.nOutHeight  = 1080;
		demoRecoder.videoConfig.nOutWidth   = 1920;
		demoRecoder.videoConfig.nSrcHeight  = 1080;
		demoRecoder.videoConfig.nSrcWidth   = 1920;
		demoRecoder.videoConfig.nBitRate    = 3*1000*1000;
		demoRecoder.videoConfig.bUsePhyBuf  = 1;
	}

	//AudioEncodeConfig audioConfig;	
	memset(&demoRecoder.audioConfig, 0x00, sizeof(AudioEncodeConfig));
	if(inputPCM != NULL){
		if(strcmp(argv[5],"pcm")==0){
			demoRecoder.audioConfig.nType		= AUDIO_ENCODE_PCM_TYPE;
		}else if(strcmp(argv[5],"aac")==0){
			demoRecoder.audioConfig.nType		= AUDIO_ENCODE_AAC_TYPE;
		}else if(strcmp(argv[5],"mp3")==0){
			demoRecoder.audioConfig.nType		= AUDIO_ENCODE_MP3_TYPE;
		}else{
			printf("the input audio encode type is not support,use the default:pcm\n");
			demoRecoder.audioConfig.nType		= AUDIO_ENCODE_PCM_TYPE;
		}
		printf("audio encode type = %d\n",demoRecoder.audioConfig.nType);
		demoRecoder.audioConfig.nInChan = 2;
		demoRecoder.audioConfig.nInSamplerate = 44100;
		demoRecoder.audioConfig.nOutChan = 2;
		demoRecoder.audioConfig.nOutSamplerate = 44100;
		demoRecoder.audioConfig.nSamplerBits = 16;
	}

    if(demoRecoder.muxType == CDX_MUXER_TS && demoRecoder.audioConfig.nType == AUDIO_ENCODE_PCM_TYPE)
    {
		demoRecoder.audioConfig.nFrameStyle = 2;
    }

    if(demoRecoder.muxType == CDX_MUXER_TS && demoRecoder.audioConfig.nType == AUDIO_ENCODE_AAC_TYPE)
    {
		demoRecoder.audioConfig.nFrameStyle = 1;//not add head when encode aac,because use ts muxer
    }
    
    if(demoRecoder.muxType == CDX_MUXER_AAC) 
    {
		demoRecoder.audioConfig.nType = AUDIO_ENCODE_AAC_TYPE;
		demoRecoder.audioConfig.nFrameStyle = 0;//add head when encode aac
    }

    if(demoRecoder.muxType == CDX_MUXER_MP3) 
    {
		demoRecoder.audioConfig.nType = AUDIO_ENCODE_MP3_TYPE;
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
	if((inputPCM != NULL) && (demoRecoder.muxType == CDX_MUXER_AAC || demoRecoder.muxType == CDX_MUXER_MP3)){
		//only encode  audio
		printf("only init audio encoder\n");
		AwEncoderInit(demoRecoder.mAwEncoder, NULL, &demoRecoder.audioConfig,&mEncDataCallBackOps);
		videoEos = 1;
    }else if((inputYUV!=NULL)&&(inputPCM==NULL)&&(demoRecoder.muxType == CDX_MUXER_MOV || demoRecoder.muxType == CDX_MUXER_TS)){
		//only encode video
		printf("only init video encoder\n");
		AwEncoderInit(demoRecoder.mAwEncoder, &demoRecoder.videoConfig, NULL,&mEncDataCallBackOps);
		audioEos = 1;
	}else if((inputYUV!=NULL)&&(inputPCM!=NULL)&&(demoRecoder.muxType == CDX_MUXER_MOV || demoRecoder.muxType == CDX_MUXER_TS)){
		//encode audio and video
		printf("init audio and video encoder\n");
		AwEncoderInit(demoRecoder.mAwEncoder, &demoRecoder.videoConfig, &demoRecoder.audioConfig,&mEncDataCallBackOps);
    }
	strcpy(demoRecoder.pUrl,argv[3]);
	printf("demoRecoder.pUrl = %s\n",demoRecoder.pUrl);
    AwEncoderStart(demoRecoder.mAwEncoder);

    AwEncoderGetExtradata(demoRecoder.mAwEncoder,&demoRecoder.extractDataBuff,&demoRecoder.extractDataLength);
#if SAVE_VIDEO_FRAME
    if(demoRecoder.fpSaveVideoFrame)
    {
		fwrite(demoRecoder.extractDataBuff, 1, demoRecoder.extractDataLength, demoRecoder.fpSaveVideoFrame);
    }
#endif

	if(inputPCM != NULL){
		printf("create audio input thread\n");
		pthread_create(&demoRecoder.audioDataThreadId, NULL, AudioInputThread, &demoRecoder);
	}
    if((inputYUV != NULL) && ((demoRecoder.muxType == CDX_MUXER_MOV) || (demoRecoder.muxType == CDX_MUXER_TS)))
    {
	printf("create video input thread\n");
		pthread_create(&demoRecoder.videoDataThreadId, NULL, VideoInputThread, &demoRecoder);
    }

    pthread_create(&demoRecoder.muxerThreadId, NULL, MuxerThread, &demoRecoder);

	if(demoRecoder.muxerThreadId)
		pthread_join(demoRecoder.muxerThreadId,     NULL);
    if(demoRecoder.audioDataThreadId)
		pthread_join(demoRecoder.audioDataThreadId, NULL);
    if(demoRecoder.videoDataThreadId)
		pthread_join(demoRecoder.videoDataThreadId, NULL);

	printf("destroy AwRecorder.\n");
    while (!CdxQueueEmpty(demoRecoder.dataQueue))
    {
        printf("free a packet\n");
        mPacket = CdxQueuePop(demoRecoder.dataQueue);
        free(mPacket->buf);
        free(mPacket);
    }
    CdxQueueDestroy(demoRecoder.dataQueue);
    AwPoolDestroy(demoRecoder.pool);
    
	if(demoRecoder.mAwEncoder != NULL)
	{
		AwEncoderStop(demoRecoder.mAwEncoder);
	    AwEncoderDestory(demoRecoder.mAwEncoder);
	    demoRecoder.mAwEncoder = NULL;
	}
	
    pthread_mutex_destroy(&demoRecoder.mMutex);

#if SAVE_VIDEO_FRAME
    if(demoRecoder.fpSaveVideoFrame)
	fclose(demoRecoder.fpSaveVideoFrame);
#endif
#if SAVE_AUDIO_FRAME
    if(demoRecoder.fpSaveAudioFrame)
	fclose(demoRecoder.fpSaveAudioFrame);
#endif

    if (inputYUV)
    {
        fclose(inputYUV);
        inputYUV = NULL;
    }

    if (inputPCM)
    {
         fclose(inputPCM);
         inputPCM = NULL;
    }
    
    printf("\n");
    printf("******************************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("******************************************************************************************\n");
    printf("\n");
	
	return 0;
}
