#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "type_camera.h"
#include "hawkview.h"

extern int JpegEnc(void * pBufOut, int * bufSize, JPEG_ENC_t *jpeg_enc);
/*
extern int ion_alloc_open();
extern int ion_alloc_close();
extern int ion_alloc_alloc(int size);
extern void ion_alloc_free(void * pbuf);
extern int ion_alloc_vir2phy(void * pbuf);
extern int ion_alloc_phy2vir(void * pbuf);
extern void ion_flush_cache(void* startAddr, int size);
extern void ion_flush_cache_all();
*/
int save_data_to_file(void* str,void* start,int length,int is_one_frame)
{
	FILE* fp; 

	if(is_one_frame)
			fp = fopen(str,"wrb+");		//save one frame data
	else			
			fp = fopen(str,"warb+");		//save more frames	//TODO: test
	if(!fp) {
			hv_err("Open file error\n");
			return -1;
	}
	if(fwrite(start,length,1,fp)){
			hv_err("Write file success!!!\n");
			fflush(fp);
			fclose(fp);
			return 0;
	}
	else {
			hv_err("Write file fail\n");
			fclose(fp);
			return -1;
	}
}

int save_jpeg_frame_by_viraddr(char* path,void* srcviraddr,int src_w,int src_h)
{
/*	int ret;
	int thumb_w = 320, thumb_h = 240;
	JPEG_ENC_t jpeg_enc;
	memset(&jpeg_enc, 0, sizeof(jpeg_enc));
	//jpeg_enc.addrY			= phy_addr;
	//jpeg_enc.addrC			= phy_addr + ALIGN_16B(src_w) * src_h;
	jpeg_enc.src_w			= src_w;
	jpeg_enc.src_h			= src_h;
	jpeg_enc.pic_w			= src_w ;
	jpeg_enc.pic_h			= src_h;
	jpeg_enc.colorFormat		= JPEG_COLOR_YUV420_NV12;
	jpeg_enc.quality		= 88;
	jpeg_enc.rotate			= 0;

	//
	strcpy(jpeg_enc.CameraMake,	"exif make test");
	strcpy(jpeg_enc.CameraModel,	"exif model test");
	strcpy(jpeg_enc.DateTime, 	"2014:02:21 10:54:05");
	
	jpeg_enc.thumbWidth	= thumb_w;
	jpeg_enc.thumbHeight	= thumb_h;
	jpeg_enc.whitebalance   = 0;

	jpeg_enc.scale_factor	= 1;
	jpeg_enc.focal_length	= 0.0;

	jpeg_enc.enable_gps	= 0;
	jpeg_enc.enable_crop	= 0;

	//cedarx_hardware_init(0);


	int frame_size = src_w * src_h * 3 >> 1;
	ion_alloc_open();
	void * vir_addr = (void*)ion_alloc_alloc(frame_size);
	unsigned int phy_addr = ion_alloc_vir2phy(vir_addr);
	//ion_flush_cache(srcviraddr,frame_size);
	memcpy(vir_addr,srcviraddr,frame_size);
	ion_flush_cache(vir_addr,frame_size);
	void * pOutBuf = (void*)malloc(src_w * src_h);
	jpeg_enc.addrY = phy_addr;
	jpeg_enc.addrC = phy_addr + ALIGN_16B(src_w) * src_h;
	int bufSize = 0;
	//save_data_to_file("data/camera/aaa",srcviraddr,frame_size,1);
	ret = JpegEnc(pOutBuf,&bufSize,&jpeg_enc);
	if (ret < 0){
		hv_err("JpegEnc failed\n");
		return -1;
	}
	hv_dbg("jpeg buffer size is %d\n",bufSize);
	save_data_to_file(path,pOutBuf,bufSize,1);
	ion_alloc_free(vir_addr);
	free(pOutBuf);
	ion_alloc_close();


	//cedarx_hardware_exit(0);
	return bufSize;
	*/
	return 0;
}
int save_jpeg_frame(char* path,unsigned int phy_addr,int src_w,int src_h)
{
	int ret;
	int thumb_w = 320, thumb_h = 240;
	hv_dbg("phy_addr = 0x%x\n",phy_addr);
	JPEG_ENC_t jpeg_enc;
	memset(&jpeg_enc, 0, sizeof(jpeg_enc));
	jpeg_enc.addrY			= phy_addr;
	jpeg_enc.addrC			= phy_addr + ALIGN_16B(src_w) * src_h;
	jpeg_enc.src_w			= src_w;
	jpeg_enc.src_h			= src_h;
	jpeg_enc.pic_w			= src_w ;
	jpeg_enc.pic_h			= src_h;
	jpeg_enc.colorFormat		= JPEG_COLOR_YUV420_NV12;
	jpeg_enc.quality		= 88;
	jpeg_enc.rotate			= 0;

	//
	strcpy(jpeg_enc.CameraMake,	"exif make test");
	strcpy(jpeg_enc.CameraModel,	"exif model test");
	strcpy(jpeg_enc.DateTime, 	"2014:02:21 10:54:05");
	
	jpeg_enc.thumbWidth	= thumb_w;
	jpeg_enc.thumbHeight	= thumb_h;
	jpeg_enc.whitebalance   = 0;

	jpeg_enc.scale_factor	= 1;
	jpeg_enc.focal_length	= 0.0;

	jpeg_enc.enable_gps	= 0;
	jpeg_enc.enable_crop	= 0;

	cedarx_hardware_init(0);

	void * pOutBuf = NULL;
	int bufSize = 0;
	pOutBuf = (void *)malloc(src_w * src_h*2);
	memset(pOutBuf,0,src_w * src_h);
	ret = JpegEnc(pOutBuf,&bufSize,&jpeg_enc);
	if (ret < 0){
		hv_err("JpegEnc failed\n");
		return -1;
	}
	hv_dbg("jpeg buffer size is %d\n",bufSize);
	save_data_to_file(path,pOutBuf,bufSize,1);
	free(pOutBuf);
	cedarx_hardware_exit(0);
	return bufSize;
}
