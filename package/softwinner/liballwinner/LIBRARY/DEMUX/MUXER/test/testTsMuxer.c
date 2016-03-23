#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vencoder.h"
#include "CdxMuxer.h"
#include <time.h>

#define OUTPUT_H264 0
#define DEMO_FILE_NAME_LEN 256

#define _ENCODER_TIME_
#ifdef _ENCODER_TIME_
static long long GetNowUs()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}

long long time1=0;
long long time2=0;
long long time3=0;
#endif


typedef struct
{
    char 			intput_file[256];
    char 			output_file[256];
    char            ts_file[256];

	unsigned int  encode_frame_num;
	unsigned int  encode_format;

	unsigned int src_size;
	unsigned int dst_size;

	unsigned int src_width;
	unsigned int src_height;
	unsigned int dst_width;
	unsigned int dst_height;

	int bit_rate;
	int frame_rate;
	int maxKeyFrame;
}encode_param_t;

typedef enum
{
    INPUT,
    HELP,
    ENCODE_FRAME_NUM,
    ENCODE_FORMAT,
    OUTPUT,
    SRC_SIZE,
    DST_SIZE,
    TS,
    INVALID
}ARGUMENT_T;

typedef struct
{
    char Short[8];
    char Name[128];
    ARGUMENT_T argument;
    char Description[512];
}argument_t;

static const argument_t ArgumentMapping[] =
{
    { "-h",  "--help",    HELP,
        "Print this help" },
	{ "-i",  "--input",   INPUT,
		"Input file path" },
	{ "-n",  "--encode_frame_num",   ENCODE_FRAME_NUM,
		"After encoder n frames, encoder stop" },
	{ "-f",  "--encode_format",  ENCODE_FORMAT,
		"0:h264 encoder, 1:jpeg_encoder" },
	{ "-o",  "--output",  OUTPUT,
		"output file path" },
	{ "-s",  "--srcsize",  SRC_SIZE,
		"src_size,can be 1080,720,480" },
	{ "-d",  "--dstsize",  DST_SIZE,
		"dst_size,can be 1080,720,480" },
	{ "-t",  "--ts",  TS,
		"output ts file path" }
};

int yu12_nv12(unsigned int width, unsigned int height, unsigned char *addr_uv, unsigned char *addr_tmp_uv)
{
	unsigned int i, chroma_bytes;
	unsigned char *u_addr = NULL;
	unsigned char *v_addr = NULL;
	unsigned char *tmp_addr = NULL;

	chroma_bytes = width*height/4;

	u_addr = addr_uv;
	v_addr = addr_uv + chroma_bytes;
	tmp_addr = addr_tmp_uv;

	for(i=0; i<chroma_bytes; i++)
	{
		*(tmp_addr++) = *(u_addr++);
		*(tmp_addr++) = *(v_addr++);
	}

	memcpy(addr_uv, addr_tmp_uv, chroma_bytes*2);	

	return 0;
}

ARGUMENT_T GetArgument(char *name)
{
    int i = 0;
    int num = sizeof(ArgumentMapping) / sizeof(argument_t);
    while(i < num)
    {
        if((0 == strcmp(ArgumentMapping[i].Name, name)) ||
			((0 == strcmp(ArgumentMapping[i].Short, name)) &&
			 (0 != strcmp(ArgumentMapping[i].Short, "--"))))
        {
            return ArgumentMapping[i].argument;
        }
        i++;
    }
    loge("INVALID name is %s\n", name);
    return INVALID;
}

static void PrintDemoUsage(void)
{
    int i = 0;
    int num = sizeof(ArgumentMapping) / sizeof(argument_t);
    printf("Usage:");
    while(i < num)
    {
        printf("%s %-32s %s", ArgumentMapping[i].Short, ArgumentMapping[i].Name,
        		ArgumentMapping[i].Description);
		printf("\n");
        i++;
    }
}

void demoParseArgument(encode_param_t *encode_param, char *argument, char *value)
{
    ARGUMENT_T arg;
    
    arg = GetArgument(argument);

	int len = 0;
	if(arg != HELP)
		strlen(value);
    if(len > DEMO_FILE_NAME_LEN)
    	return;
	
    switch(arg)
    {
        case HELP:
        	PrintDemoUsage();
            exit(-1);
        case INPUT:
			memset(encode_param->intput_file, 0, sizeof(encode_param->intput_file));
        	sscanf(value, "%255s", encode_param->intput_file);
        	logd(" get input file: %s ", encode_param->intput_file);
            break;
        case ENCODE_FRAME_NUM:
        	sscanf(value, "%u", &encode_param->encode_frame_num);
            break;
        case ENCODE_FORMAT:
        	sscanf(value, "%u", &encode_param->encode_format);
            break;
        case OUTPUT:
			memset(encode_param->output_file, 0, sizeof(encode_param->output_file));
        	sscanf(value, "%255s", encode_param->output_file);
        	logd(" get output file: %s ", encode_param->output_file);
            break;
		case SRC_SIZE:
        	sscanf(value, "%u", &encode_param->src_size);
        	logd(" get src_size: %dp ", encode_param->src_size);
			if(encode_param->src_size == 1080)
			{
				encode_param->src_width = 1920;
				encode_param->src_height = 1080;
			}
			else if(encode_param->src_size == 720)
			{
				encode_param->src_width = 1280;
				encode_param->src_height = 720;
			}
			else if(encode_param->src_size == 480)
			{
				encode_param->src_width = 640;
				encode_param->src_height = 480;
			}
			else
			{
				encode_param->src_width = 1280;
				encode_param->src_height = 720;
				logw("encoder demo only support the size 1080p,720p,480p, now use the default size 720p\n");
			}
            break;
		case DST_SIZE:
        	sscanf(value, "%u", &encode_param->dst_size);
        	logd(" get dst_size: %dp ", encode_param->dst_size);
			if(encode_param->dst_size == 1080)
			{
				encode_param->dst_width = 1920;
				encode_param->dst_height = 1080;
			}
			else if(encode_param->dst_size == 720)
			{
				encode_param->dst_width = 1280;
				encode_param->dst_height = 720;
			}
			else if(encode_param->dst_size == 480)
			{
				encode_param->dst_width = 640;
				encode_param->dst_height = 480;
			}
			else
			{
				encode_param->dst_width = 1280;
				encode_param->dst_height = 720;
				logw("encoder demo only support the size 1080p,720p,480p, now use the default size 720p\n");
			}
            break;
        case TS:
            strcpy(encode_param->ts_file, "write://");
            memset((encode_param->ts_file + 8), 0, (sizeof(encode_param->ts_file) - 8));
        	sscanf(value, "%255s", (encode_param->ts_file + 8));
        	logd(" get ts file: %s ", encode_param->ts_file);
            break;
        case INVALID:
        default:
            logd("unknowed argument :  %s", argument);
            break;
    }
}

void DemoHelpInfo(void)
{
	logd(" ==== CedarX2.0 encoder demo help start ===== ");
	logd(" -h or --help to show the demo usage");
	logd(" demo created by yangcaoyuan, allwinnertech/AL3 ");
	logd(" email: yangcaoyuan@allwinnertech.com ");
	logd(" ===== CedarX2.0 encoder demo help end ====== ");
}

int main(int argc, char** argv)
{
    
	VencBaseConfig baseConfig;
	VencAllocateBufferParam bufferParam;
	VideoEncoder* pVideoEnc = NULL;
	VencInputBuffer inputBuffer;
	VencOutputBuffer outputBuffer;
	unsigned char *uv_tmp_buffer = NULL;
	int result = 0;
	int i = 0;
	long long pts = 0;	
	FILE *in_file = NULL;
#if OUTPUT_H264
	FILE *out_file = NULL;
#endif
	char *input_path = NULL;
	char *output_path = NULL;
	char *ts_path = NULL;
	encode_param_t	encode_param;

	/********************** Define H264 Paramerter *************************/
    VencHeaderData sps_pps_data;
	VencH264Param h264Param;
	/********************** Define H264 Paramerter *************************/

	/********************** Define JPEG Paramerter *************************/
	EXIFInfo exifinfo;
	int quality = 90;
	int jpeg_mode = 1;
	/********************** Define JPEG Paramerter *************************/

	/****************************** Define MUXER Paramerter ********************************************/
	CdxDataSourceT ts_stream;
	CdxMuxerMediaInfoT media_info;
	CdxStreamT *ts_writer = NULL;
    CdxMuxerT *mux = NULL;
	/****************************** Define MUXER Paramerter ********************************************/
	
	memset(&encode_param, 0, sizeof(encode_param));	
	encode_param.src_width = 1280;
	encode_param.src_height = 720;
	encode_param.dst_width = 1280;
	encode_param.dst_height = 720;
	encode_param.bit_rate = 6 * 1024 * 1024;
	encode_param.frame_rate = 30;
	encode_param.maxKeyFrame = 30;
	encode_param.encode_format = VENC_CODEC_H264;
	encode_param.encode_frame_num = 200;
	strcpy((char*)encode_param.intput_file,		"/mnt/sdcard/DCIM/Camera/720p.yuv");
	strcpy((char*)encode_param.output_file,		"/mnt/sdcard/DCIM/Camera/720p.264");
	strcpy((char*)encode_param.ts_file,		"write:///mnt/sdcard/DCIM/Camera/720p_h264.ts");
	

	//parse the config paramter
	if(argc >= 2)
    {
        for(i = 1; i < (int)argc; i += 2)
        {
			demoParseArgument(&encode_param, argv[i], argv[i + 1]);
        }
    }
    else
    {
    	printf(" we need more arguments ");
    	PrintDemoUsage();
    	return 0;
    }

    input_path = encode_param.intput_file;
	output_path = encode_param.output_file;
	ts_path = encode_param.ts_file;
	
	in_file = fopen(input_path, "rb");
	if(in_file == NULL)
	{
		loge("open in_file fail\n");
		return -1;
	}
#if OUTPUT_H264		
	out_file = fopen(output_path, "wb");
	if(out_file == NULL)
	{
		loge("open out_file fail\n");
		fclose(in_file);
		return -1;
	}
#endif
	memset(&baseConfig, 0 ,sizeof(VencBaseConfig));
	memset(&bufferParam, 0 ,sizeof(VencAllocateBufferParam));

	baseConfig.nInputWidth= encode_param.src_width;
	baseConfig.nInputHeight = encode_param.src_height;
	baseConfig.nStride = encode_param.src_width;
	
	baseConfig.nDstWidth = encode_param.dst_width;
	baseConfig.nDstHeight = encode_param.dst_height;
	baseConfig.eInputFormat = VENC_PIXEL_YUV420P;

	bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
	bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
	bufferParam.nBufferNum = 4;
	
	/******************************* Set H264 Parameters ****************************/
	h264Param.bEntropyCodingCABAC = 1;
	h264Param.nBitrate = encode_param.bit_rate;
	h264Param.nFramerate = encode_param.frame_rate; 
	h264Param.nCodingMode = VENC_FRAME_CODING;	
	h264Param.nMaxKeyInterval = encode_param.maxKeyFrame;
	h264Param.sProfileLevel.nProfile = VENC_H264ProfileMain;
	h264Param.sProfileLevel.nLevel = VENC_H264Level31;
	h264Param.sQPRange.nMinqp = 10;
	h264Param.sQPRange.nMaxqp = 40;
	/******************************* Set H264 Parameters ****************************/

    /******************************* Set JPEG Parameters ****************************/
	exifinfo.ThumbWidth = 176;
	exifinfo.ThumbHeight = 144;
	strcpy((char*)exifinfo.CameraMake,		"allwinner make test");
	strcpy((char*)exifinfo.CameraModel,		"allwinner model test");
	strcpy((char*)exifinfo.DateTime, 		"2014:02:21 10:54:05");
	strcpy((char*)exifinfo.gpsProcessingMethod,  "allwinner gps");
	exifinfo.Orientation = 0;
	exifinfo.ExposureTime.num = 2;
	exifinfo.ExposureTime.den = 1000;
	exifinfo.FNumber.num = 20;
	exifinfo.FNumber.den = 10;
	exifinfo.ISOSpeed = 50;
	exifinfo.ExposureBiasValue.num= -4;
	exifinfo.ExposureBiasValue.den= 1;
	exifinfo.MeteringMode = 1;
	exifinfo.FlashUsed = 0;
	exifinfo.FocalLength.num = 1400;
	exifinfo.FocalLength.den = 100;
	exifinfo.DigitalZoomRatio.num = 4;
	exifinfo.DigitalZoomRatio.den = 1;
	exifinfo.WhiteBalance = 1;
	exifinfo.ExposureMode = 1;
	exifinfo.enableGpsInfo = 1;
	exifinfo.gps_latitude = 23.2368;
	exifinfo.gps_longitude = 24.3244;
	exifinfo.gps_altitude = 1234.5;
	exifinfo.gps_timestamp = (long)time(NULL);
	strcpy((char*)exifinfo.CameraSerialNum,  "123456789");
	strcpy((char*)exifinfo.ImageName,  "exif-name-test");
	strcpy((char*)exifinfo.ImageDescription,  "exif-descriptor-test");
	/******************************* Set JPEG Parameters ****************************/
	
	/******************************* Set Muxer Parameters ****************************/
	memset(&media_info, 0, sizeof(CdxMuxerMediaInfoT));
    media_info.audioNum = 0;
    media_info.videoNum = 1;
    media_info.video.nWidth = encode_param.dst_width;
    media_info.video.nHeight = encode_param.dst_height;
    media_info.video.nFrameRate = encode_param.frame_rate;
    media_info.video.eCodeType = encode_param.encode_format;
       
    memset(&ts_stream, 0, sizeof(CdxDataSourceT));

    ts_stream.uri = strdup(ts_path);
    result = CdxStreamOpen(&ts_stream, NULL, NULL, &ts_writer, NULL);
    if(result < 0)
    {
    	loge("CdxStreamOpen failed");
    	return -1;
    }
    mux = CdxMuxerCreate(CDX_MUXER_TS, ts_writer);
    CdxMuxerSetMediaInfo(mux, &media_info);
    //CdxMuxerWriteHeader(mux);
	/******************************* Set Muxer Parameters ****************************/
	
	pVideoEnc = VideoEncCreate(encode_param.encode_format);

	
	if(encode_param.encode_format == VENC_CODEC_JPEG)
	{
	  	VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegExifInfo, &exifinfo);
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegQuality, &quality);
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegEncMode, &jpeg_mode);
	}
	else if(encode_param.encode_format == VENC_CODEC_H264)
	{	
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264Param, &h264Param);
	}

	VideoEncInit(pVideoEnc, &baseConfig);

	if(encode_param.encode_format == VENC_CODEC_H264)
	{
		VideoEncGetParameter(pVideoEnc, VENC_IndexParamH264SPSPPS, &sps_pps_data);
		CdxMuxerWriteExtraData(mux, sps_pps_data.pBuffer, sps_pps_data.nLength, 0);
#if OUTPUT_H264
		fwrite(sps_pps_data.pBuffer, 1, sps_pps_data.nLength, out_file);
#endif
		logd("sps_pps_data.nLength: %d", sps_pps_data.nLength);
	}
	

	AllocInputBuffer(pVideoEnc, &bufferParam);

	if(baseConfig.eInputFormat == VENC_PIXEL_YUV420SP)
	{
		uv_tmp_buffer = (unsigned char*)malloc(baseConfig.nInputWidth*baseConfig.nInputHeight/2);
		if(uv_tmp_buffer == NULL)
		{
			loge("malloc uv_tmp_buffer fail\n");
#if OUTPUT_H264
			fclose(out_file);
#endif
			fclose(in_file);
			return -1;
		}
	}

#if USE_SVC
	// used for throw frame test with SVC
	int TemporalLayer = -1;
	char p9bytes[9] = {0};
#endif
		
	unsigned int testNumber = 0;
	
	while(testNumber < encode_param.encode_frame_num)
	{
	    CdxMuxerPacketT pkt;
		memset(&pkt, 0, sizeof(CdxMuxerPacketT));
		
		GetOneAllocInputBuffer(pVideoEnc, &inputBuffer);
		{
			unsigned int size1, size2;
			
			size1 = fread(inputBuffer.pAddrVirY, 1, baseConfig.nInputWidth*baseConfig.nInputHeight, in_file);
			size2 = fread(inputBuffer.pAddrVirC, 1, baseConfig.nInputWidth*baseConfig.nInputHeight/2, in_file);

			if((size1!= baseConfig.nInputWidth*baseConfig.nInputHeight) || (size2!= baseConfig.nInputWidth*baseConfig.nInputHeight/2))
			{
				fseek(in_file, 0, SEEK_SET);
				size1 = fread(inputBuffer.pAddrVirY, 1, baseConfig.nInputWidth*baseConfig.nInputHeight, in_file);
				size2 = fread(inputBuffer.pAddrVirC, 1, baseConfig.nInputWidth*baseConfig.nInputHeight/2, in_file);
			}

			
			if(baseConfig.eInputFormat == VENC_PIXEL_YUV420SP)
			{
				yu12_nv12(baseConfig.nInputWidth, baseConfig.nInputHeight, inputBuffer.pAddrVirC, uv_tmp_buffer);
			}
		}
		
		inputBuffer.bEnableCorp = 0;
		inputBuffer.sCropInfo.nLeft =  240;
		inputBuffer.sCropInfo.nTop  =  240;
		inputBuffer.sCropInfo.nWidth  =  240;
		inputBuffer.sCropInfo.nHeight =  240;

		FlushCacheAllocInputBuffer(pVideoEnc, &inputBuffer);

		pts += 1.0 / encode_param.frame_rate * 1000000;
		inputBuffer.nPts = pts;
		
		AddOneInputBuffer(pVideoEnc, &inputBuffer);
		time1 = GetNowUs();
		VideoEncodeOneFrame(pVideoEnc);
		time2 = GetNowUs();
		logv("encode frame %d use time is %lldus..\n",testNumber,(time2-time1));
		time3 += time2-time1;

		AlreadyUsedInputBuffer(pVideoEnc,&inputBuffer);
		ReturnOneAllocInputBuffer(pVideoEnc, &inputBuffer);

		result = GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
		if(result == -1)
		{
			goto out;
		}

		/*************************** Write Packet **********************************/
		pkt.buflen = outputBuffer.nSize0 + outputBuffer.nSize1;
        if ((pkt.buf = (char*)malloc(pkt.buflen)) == NULL)
        {
            loge("pkt.buf malloc failed in No. %d frame\n", testNumber);
            break;
        }
        memcpy(pkt.buf, outputBuffer.pData0, outputBuffer.nSize0);
        
        if (outputBuffer.nSize1 > 0)
        {
            memcpy((char*)pkt.buf + outputBuffer.nSize0, outputBuffer.pData1, outputBuffer.nSize1);
        }
        
        pkt.pts = outputBuffer.nPts / 1000; // us should be changed to ms
        pkt.duration = 1.0/ media_info.video.nFrameRate * 1000;
        pkt.type = 0;
        pkt.streamIndex = 0;
        
        result = CdxMuxerWritePacket(mux, &pkt);

        if (result)
        {
            loge("CdxMuxerWritePacket() failed in No. %d frame\n", testNumber);
            break;
        }
        free(pkt.buf);
        pkt.buf = NULL;
		/*************************** Write Packet **********************************/
#if OUTPUT_H264		
		fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);
		if(outputBuffer.nSize1)
		{
			fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);
		}
#endif
		FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);

		if(h264Param.nCodingMode==VENC_FIELD_CODING && encode_param.encode_format==VENC_CODEC_H264)
		{
			GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
#if OUTPUT_H264
			fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);

			if(outputBuffer.nSize1)
			{
				fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);
			}
#endif				
			FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);
		}

		testNumber++;
	}
	
    result = CdxMuxerWriteTrailer(mux);
    if (result)
    {
        loge("CdxMuxerWriteTrailer() failed\n");
    }
	logd("the average encode time is %lldus...\n",time3/testNumber);

out:
	printf("output file is saved: %s and %s\n",encode_param.output_file, (encode_param.ts_file + 8));
#if OUTPUT_H264
	fclose(out_file);
	out_file = NULL;
#endif
	fclose(in_file);
	in_file = NULL;
	if(uv_tmp_buffer)
	{
		free(uv_tmp_buffer);
	}

	result = VideoEncUnInit(pVideoEnc);
	if( result )
	{
		loge("VideoEncUnInit error result=%d...\n",result);		
	}
	
	VideoEncDestroy(pVideoEnc);
	pVideoEnc = NULL;	

	CdxMuxerClose(mux);

	if(ts_stream.uri)
	{
	    free(ts_stream.uri);
	    ts_stream.uri = NULL;
	}
		
	return 0;
}

