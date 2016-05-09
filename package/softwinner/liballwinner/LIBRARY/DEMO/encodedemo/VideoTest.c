#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vencoder.h"
#include <sys/time.h>
#include "memoryAdapter.h"

#define DEMO_FILE_NAME_LEN 256
#define	USE_SVC	0


typedef struct
{
    char			intput_file[256];
    char			output_file[256];

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

void ParseArgument(encode_param_t *encode_param, char *argument, char *value)
{
    ARGUMENT_T arg;
    
    arg = GetArgument(argument);

	int len = 0;
	if(arg != HELP)
		len = strlen(value);
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

int SeekPrefixNAL(char* begin)
{	
	unsigned int i;
	char* pchar = begin;
	char TemporalID = 0;
	char isPrefixNAL = 1;
	char NAL[4] = {0x00, 0x00, 0x00, 0x01};
	char PrefixNAL[3] = {0x6e, 0x4e, 0x0e};

	if(!pchar)
	{
		return -1;
	}
	
	for(i=0; i<4; i++)
	{
		if(pchar[i] != NAL[i])
		{
			isPrefixNAL = 0;
			break;
		}
	}
	if(isPrefixNAL == 1)
	{	
		isPrefixNAL = 0;
		for(i=0; i<3; i++)
		{
			if(pchar[4] == PrefixNAL[i])
			{
				isPrefixNAL = 1;
				break;
			}
		}
	}
	// read temporal_id
	if(isPrefixNAL == 1)
	{
		TemporalID = pchar[7];
		TemporalID >>= 5;
		return TemporalID;
	}

	return -1;
}

int main(int argc, char** argv)
{
	VencBaseConfig baseConfig;
	VencAllocateBufferParam bufferParam;
	VideoEncoder* pVideoEnc = NULL;
	VencInputBuffer inputBuffer;
	VencOutputBuffer outputBuffer;
	VencHeaderData sps_pps_data;
	VencH264Param h264Param;
	//VencH264FixQP fixQP;
	EXIFInfo exifinfo;
	//VencCyclicIntraRefresh sIntraRefresh;
	unsigned char *uv_tmp_buffer = NULL;

	VencSuperFrameConfig	sSuperFrameCfg;
	//VencH264AspectRatio		sAspectRatio;

	struct ScMemOpsS* memops = MemAdapterGetOpsS();
    if(memops == NULL)
    {
	printf("MemAdapterGetOpsS failed\n");
	return -1;
    }
    CdcMemOpen(memops);

	int result = 0;
	int i = 0;
	
	FILE *in_file = NULL;
	FILE *out_file = NULL;

	char *input_path = NULL;
	char *output_path = NULL;

	//set the default encode param
	encode_param_t	encode_param;
	memset(&encode_param, 0, sizeof(encode_param));
	
	encode_param.src_width = 1280;
	encode_param.src_height = 720;
	encode_param.dst_width = 1280;
	encode_param.dst_height = 720;

	encode_param.bit_rate = 2*1024*1024;
	encode_param.frame_rate = 30;
	encode_param.maxKeyFrame = 30;

	encode_param.encode_format = VENC_CODEC_H264;
	encode_param.encode_frame_num = 200;

	strcpy((char*)encode_param.intput_file,		"/mnt/720p.yuv");
	strcpy((char*)encode_param.output_file,		"/mnt/720p.264");

	//parse the config paramter
	if(argc >= 2)
    {
        for(i = 1; i < (int)argc; i += 2)
        {
			ParseArgument(&encode_param, argv[i], argv[i + 1]);
        }
    }
    else
    {
	printf(" we need more arguments ");
	PrintDemoUsage();
	return 0;
    }
	
#if 0
	// roi
	VencROIConfig sRoiConfig[4];
	
	sRoiConfig[0].bEnable = 1;
	sRoiConfig[0].index = 0;
	sRoiConfig[0].nQPoffset = 10;
	sRoiConfig[0].sRect.nLeft = 320;
	sRoiConfig[0].sRect.nTop = 180;
	sRoiConfig[0].sRect.nWidth = 320;
	sRoiConfig[0].sRect.nHeight = 180;

	sRoiConfig[1].bEnable = 1;
	sRoiConfig[1].index = 1;
	sRoiConfig[1].nQPoffset = 10;
	sRoiConfig[1].sRect.nLeft = 320;
	sRoiConfig[1].sRect.nTop = 180;
	sRoiConfig[1].sRect.nWidth = 320;
	sRoiConfig[1].sRect.nHeight = 180;

	sRoiConfig[2].bEnable = 1;
	sRoiConfig[2].index = 2;
	sRoiConfig[2].nQPoffset = 10;
	sRoiConfig[2].sRect.nLeft = 320;
	sRoiConfig[2].sRect.nTop = 180;
	sRoiConfig[2].sRect.nWidth = 320;
	sRoiConfig[2].sRect.nHeight = 180;

	sRoiConfig[3].bEnable = 1;
	sRoiConfig[3].index = 3;
	sRoiConfig[3].nQPoffset = 10;
	sRoiConfig[3].sRect.nLeft = 320;
	sRoiConfig[3].sRect.nTop = 180;
	sRoiConfig[3].sRect.nWidth = 320;
	sRoiConfig[3].sRect.nHeight = 180;
#endif
	//intraRefresh
	//sIntraRefresh.bEnable = 1;
	//sIntraRefresh.nBlockNumber = 10;

	//fix qp mode
	//fixQP.bEnable = 1;
	//fixQP.nIQp = 20;
	//fixQP.nPQp = 30;

	//* h264 param
	h264Param.bEntropyCodingCABAC = 1;
	h264Param.nBitrate = encode_param.bit_rate;
	h264Param.nFramerate = encode_param.frame_rate; 
	h264Param.nCodingMode = VENC_FRAME_CODING;
	//h264Param.nCodingMode = VENC_FIELD_CODING;
	
	h264Param.nMaxKeyInterval = encode_param.maxKeyFrame;
	h264Param.sProfileLevel.nProfile = VENC_H264ProfileMain;
	h264Param.sProfileLevel.nLevel = VENC_H264Level31;
	h264Param.sQPRange.nMinqp = 10;
	h264Param.sQPRange.nMaxqp = 40;

	exifinfo.ThumbWidth = 176;
	exifinfo.ThumbHeight = 144;

	strcpy((char*)exifinfo.CameraMake,		"allwinner make test");
	strcpy((char*)exifinfo.CameraModel,		"allwinner model test");
	strcpy((char*)exifinfo.DateTime,		"2014:02:21 10:54:05");
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
	
	exifinfo.gps_timestamp = 0; //(long)time(NULL);

	strcpy((char*)exifinfo.CameraSerialNum,  "123456789");
	strcpy((char*)exifinfo.ImageName,  "exif-name-test");
	strcpy((char*)exifinfo.ImageDescription,  "exif-descriptor-test");
	input_path = encode_param.intput_file;
	output_path = encode_param.output_file;
	
	in_file = fopen(input_path, "r");
	if(in_file == NULL)
	{
		loge("open in_file fail\n");
		return -1;
	}
		
	out_file = fopen(output_path, "wb");
	if(out_file == NULL)
	{
		loge("open out_file fail\n");
		fclose(in_file);
		return -1;
	}

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
	bufferParam.nBufferNum = 1; 
	
	pVideoEnc = VideoEncCreate(encode_param.encode_format);

	int quality = 90;
	//int jpeg_mode = 1;
	if(encode_param.encode_format == VENC_CODEC_JPEG)
	{
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegExifInfo, &exifinfo);
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegQuality, &quality);
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegEncMode, &jpeg_mode);
	}
	else if(encode_param.encode_format == VENC_CODEC_H264)
	{
		int value;
		
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264Param, &h264Param);

		value = 0;
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamIfilter, &value);

		value = 0; //degree
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamRotation, &value);

		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264FixQP, &fixQP);

		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264CyclicIntraRefresh, &sIntraRefresh);

		//value = 720/4;
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamSliceHeight, &value);

		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[0]);
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[1]);
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[2]);
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[3]);
		value = 0;
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetPSkip, &value);

		//sAspectRatio.aspect_ratio_idc = 255;
		//sAspectRatio.sar_width = 4;
		//sAspectRatio.sar_height = 3;
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264AspectRatio, &sAspectRatio);
		
		//value = 1;
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264FastEnc, &value);
		
		if(0)
		{
			//sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_REENCODE;
			sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_NONE;
			sSuperFrameCfg.nMaxIFrameBits = 30000*8;
			sSuperFrameCfg.nMaxPFrameBits = 15000*8;
			VideoEncSetParameter(pVideoEnc, VENC_IndexParamSuperFrameConfig, &sSuperFrameCfg);
		}
	}

	int vbvSize = 4*1024*1024;
	VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetVbvSize, &vbvSize);

	baseConfig.memops = memops;
	VideoEncInit(pVideoEnc, &baseConfig);

	if(encode_param.encode_format == VENC_CODEC_H264)
	{
		unsigned int head_num = 0;
		VideoEncGetParameter(pVideoEnc, VENC_IndexParamH264SPSPPS, &sps_pps_data);
		fwrite(sps_pps_data.pBuffer, 1, sps_pps_data.nLength, out_file);
		logd("sps_pps_data.nLength: %d", sps_pps_data.nLength);
		for(head_num=0; head_num<sps_pps_data.nLength; head_num++)
			logd("the sps_pps :%02x\n", *(sps_pps_data.pBuffer+head_num));
	}
	
	if(encode_param.encode_format == VENC_CODEC_JPEG)
	{
		//encode_param.encode_frame_num = 1;
	}

	//AllocInputBuffer(pVideoEnc, &bufferParam);


	unsigned char* pAddrVirY = CdcMemPalloc(memops, bufferParam.nSizeY);
	unsigned char* pAddrVirC = CdcMemPalloc(memops, bufferParam.nSizeC);
			
	fread(pAddrVirY, 1, baseConfig.nInputWidth*baseConfig.nInputHeight, in_file);
	fread(pAddrVirC, 1, baseConfig.nInputWidth*baseConfig.nInputHeight/2, in_file);
	CdcMemFlushCache(memops, pAddrVirY, bufferParam.nSizeY);
	CdcMemFlushCache(memops, pAddrVirC, bufferParam.nSizeC);

	inputBuffer.nID = 0;
	inputBuffer.pAddrPhyY = CdcMemGetPhysicAddressCpu(memops, pAddrVirY);
	inputBuffer.pAddrPhyC = CdcMemGetPhysicAddressCpu(memops, pAddrVirC);
			
	unsigned int testNumber = 0;
		
	while(testNumber < encode_param.encode_frame_num)
	{
		inputBuffer.bEnableCorp = 0;
		inputBuffer.sCropInfo.nLeft =  240;
		inputBuffer.sCropInfo.nTop  =  240;
		inputBuffer.sCropInfo.nWidth  =  240;
		inputBuffer.sCropInfo.nHeight =  240;

		AddOneInputBuffer(pVideoEnc, &inputBuffer);
		VideoEncodeOneFrame(pVideoEnc);

		AlreadyUsedInputBuffer(pVideoEnc,&inputBuffer);

		result = GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
		if((sSuperFrameCfg.eSuperFrameMode==VENC_SUPERFRAME_DISCARD) && (result==-1))
		{
			logd("VENC_SUPERFRAME_DISCARD: discard frame %d\n",testNumber);
			continue; 
		}
		else if(result == -1)
		{
			goto out;
		}
		
		fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);

		if(outputBuffer.nSize1)
		{
			fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);
		}

		FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);

		if(h264Param.nCodingMode==VENC_FIELD_CODING && encode_param.encode_format==VENC_CODEC_H264)
		{
			GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
			//logi("size: %d,%d", outputBuffer.nSize0,outputBuffer.nSize1);

			fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);

			if(outputBuffer.nSize1)
			{
				fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);
			}
				
			FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);
		}

		testNumber++;
	}

out:
	printf("output file is saved:%s\n",encode_param.output_file);
	fclose(out_file);
	fclose(in_file);
	if(uv_tmp_buffer)
		free(uv_tmp_buffer);

	
	if(pAddrVirY)
		CdcMemPfree(memops, pAddrVirY);
	if(pAddrVirC)
		CdcMemPfree(memops, pAddrVirC);
	if(memops) CdcMemClose(memops);

	return 0;
}
