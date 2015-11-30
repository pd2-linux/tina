#include "video_helper.h"
#include "path.h"
extern struct buffer *buffers;

int is_x_msec(int msec,long long secs,long long usecs)
{
	static long long timestamp_old = 0;
	long long timestamp = usecs + secs* 1000000;
	if(timestamp_old == 0)
		timestamp_old = timestamp;
	if((timestamp - timestamp_old) > msec*1000){
		timestamp_old = timestamp;
		return 1;
	}	
	return 0;
	
}
float get_framerate(long long secs,long long usecs)
{
	static long long timestamp_old = 0;
	long long timestamp;
	long long rate;
	timestamp = usecs + secs* 1000000;
	rate = timestamp - timestamp_old;
	if(rate == 0 || timestamp_old == 0){
		timestamp_old = timestamp;
		return 0.0;
	}

	//hv_dbg("rate: %0.2f\n",1000000/(float)rate);
	timestamp_old = timestamp;
	return 1000000/(float)rate;
}
int write_file(char* file_path,char* string,int lenght )
{
	FILE* fp;
	fp = fopen(file_path,"wrb+");
	if(!fp) {
			hv_err("Open sync file error");
			return -1;
	}	
	
	//hv_dbg("%s: %s\n",file_path,string);
	if(fwrite(string,lenght,1,fp)){
			fclose(fp);
			return 0;
	}
	else{
			hv_err("Write file fail\n");
			fclose(fp);
			return -1;
	}
}
int make_exif_info(char* exif_str,char* name, struct isp_exif_attribute *exif,int w,int h)
{
	sprintf(exif_str,					\
				 "image_name       = %s\n" 	\
				 "width            = %d\n"	\
				 "height           = %d\n"	\
				 "exp_time_num     = %d\n"	\
				 "exp_time_den     = %d\n"	\
				 "sht_speed_num    = %d\n"	\
				 "sht_speed_den    = %d\n"	\
				 "fnumber          = %d\n"	\
				 "exp_bias         = %d\n"	\
				 "foc_length       = %d\n"	\
				 "iso_speed        = %d\n"	\
				 "flash_fire       = %d\n"	\
				 "brightness       = %d\n#",	\
	name,			\
	w,						\
	h,						\
	exif->exposure_time.numerator,	\
	exif->exposure_time.denominator,\
	exif->shutter_speed.numerator,	\
	exif->shutter_speed.denominator,\
	exif->fnumber,					\
	exif->exposure_bias,			\
	exif->focal_length,				\
	exif->iso_speed,				\
	exif->flash_fire,				\
	exif->brightness);
	return 0;
}
int set_exif_info(void* capture){
	char exif_str[1000];
	char file_path[50];
	capture_handle* cap = (capture_handle*)capture;
	struct isp_exif_attribute *exif = &(cap->picture.exif);
	memset(exif_str,0,sizeof(exif_str));
	sprintf(file_path,"%s/%s.exif",PATH,cap->picture.path_name);
	exif = &(cap->picture.exif);
	make_exif_info(exif_str,cap->picture.path_name,exif,cap->cap_w,cap->cap_h);
	hv_dbg("image exif info:\n%s\n",exif_str);
	return write_file(file_path,exif_str,sizeof(exif_str));;
}

int set_cap_info(void* capture)
{
	char info[500];
	char exif[500];
	char file_path[20];
	capture_handle* cap = (capture_handle*)capture;
	memset(info,0,sizeof(info));
	strcpy(file_path,"dev/info");
	//sync string: sensor_type:save_status:framrate:capture_w:capture_h,sub_w,sub_h#
	sprintf(info,	\
				 "sensor_type      = %s\n" 		\
				 "status           = %d\n"		\
				 "framerate        = %0.2f\n"	\
				 "subchanel_width  = %d\n"		\
				 "subchanel_height = %d\n"		\
				 "rotation         = %d\n\n",		\
			 (cap->sensor_type == 1)?"raw":"yuv",	\
			 cap->save_status,	\
			 cap->cap_fps,		\
			 cap->sub_w,		\
			 cap->sub_h,		\
			 cap->sub_rot);

	make_exif_info(exif,"none",&(cap->frame.exif),cap->cap_w,cap->cap_h);
	strcat(info,exif);
	//hv_dbg("info str:\n%s\n",info);
	return write_file(file_path,info,sizeof(info));
}
int set_sync_status(void* capture,int index)
{
	char sync[5];
	char file_path[20];
	
	memset(sync,0,sizeof(sync));

	sprintf(sync,"%d", index);
	strcpy(file_path,"dev/sync");
	
	return write_file(file_path,sync,sizeof(sync));
	
}

int save_frame_to_file(void* str,void* start,int w,int h,int format,int is_one_frame)
{
	FILE* fp; 
	int length;

	length = w*h*3>>1;

	if(is_one_frame)
			fp = fopen(str,"wrb+");		//save one frame data
	else			
			fp = fopen(str,"warb+");		//save more frames	//TODO: test
	if(!fp) {
			hv_err("Open file error\n");
			return -1;
	}
	if(fwrite(start,length,1,fp)){
			fclose(fp);
			return 0;
	}
	else {
			hv_err("Write file fail\n");
			fclose(fp);
			return -1;
	}
}
int get_disp_addr(void* capture,unsigned int origin,unsigned int* addr,int* w,int* h)
{
	unsigned int addrPhyY;
	capture_handle* cap = (capture_handle*)capture;
	addrPhyY = origin;
	if(cap->sensor_type == V4L2_SENSOR_TYPE_RAW){
		addrPhyY = addrPhyY + ALIGN_4K(ALIGN_16B(cap->cap_w) * cap->cap_h * 3 >> 1);
		*h = cap->sub_h;
		*w = cap->sub_w;
	}else{
		*h = cap->cap_h;
		*w = cap->cap_w;
	}
	if(cap->sub_rot == 90 || cap->sub_rot == 270){
		int tmp;
		addrPhyY = addrPhyY + ALIGN_4K(ALIGN_16B(*w) * *h * 3 >> 1);
		tmp = *h;
		*h = *w;
		*w = tmp;
	}
	*addr = addrPhyY;
	return 0;
}
#ifdef ANDROID_ENV
extern int save_jpeg_frame(char* path,unsigned int phy_addr,int src_w,int src_h);
extern int save_jpeg_frame_by_viraddr(char* path,void* srcviraddr,int src_w,int src_h);
#endif
int do_save_image(void* capture,int buf_index)
{
	int ret;
	char image_name[30];

	capture_handle* cap = (capture_handle*)capture;
	memset(image_name,0,sizeof(image_name));
	sprintf(image_name,"%s/%s",PATH, cap->picture.path_name);
	hv_dbg("image_name: %s\n",image_name);

	set_exif_info(capture);
	hv_dbg("--------set_exif_info end\n");
#ifdef ANDROID_ENV
	ret = save_jpeg_frame(image_name,buffers[buf_index].phy_addr,cap->cap_w,cap->cap_h);
	//ret = save_jpeg_frame_by_viraddr(image_name,(void*)(buffers[buf_index].start),cap->cap_w,cap->cap_h);

#else
        sprintf(image_name,"%s/yuv%s", PATH,cap->picture.path_name);
	ret = save_frame_to_file(image_name,				\
				  (void*)(buffers[buf_index].start),	\
				  cap->cap_w,cap->cap_h,cap->cap_fmt,	\
				  1);
#endif
	if(ret == -1)
		hv_err("save image failed!\n");
	return 0;
}
int do_save_sub_image(void* capture,int buf_index)
{
	int ret;
	char image_name[30];

	capture_handle* cap = (capture_handle*)capture;
	memset(image_name,0,sizeof(image_name));
	sprintf(image_name,"%s/%s", PATH,cap->picture.path_name);
	hv_dbg("image_name: %s\n",image_name);

	set_exif_info(capture);
	hv_dbg("--------set_exif_info end\n");
	void* vir_sub_start = NULL;
	unsigned int phy_sub_start = 0;
	int w,h;
	if(cap->sensor_type == V4L2_SENSOR_TYPE_RAW){
		vir_sub_start = (unsigned int)(buffers[buf_index].start) + ALIGN_4K(ALIGN_16B(cap->cap_w) * cap->cap_h * 3 >> 1);
		phy_sub_start = buffers[buf_index].phy_addr + ALIGN_4K(ALIGN_16B(cap->cap_w) * cap->cap_h * 3 >> 1);
		w = cap->sub_w;
		h = cap->sub_h;
	}
	else {
		vir_sub_start = buffers[buf_index].start;
		phy_sub_start = buffers[buf_index].phy_addr;
		w = cap->cap_w;
		h = cap->cap_h;
	}
#ifdef ANDROID_ENV
	ret = save_jpeg_frame(image_name,phy_sub_start,w,h);
	//sprintf(image_name,"/data/camera/yuv%s", cap->picture.path_name);
	//ret = save_jpeg_frame_by_viraddr(image_name,(void*)vir_sub_start,cap->sub_w,cap->sub_h);
#else
        sprintf(image_name,"%s/yuv%s", PATH,cap->picture.path_name);
	ret = save_frame_to_file(image_name,				\
				  (void*)(vir_sub_start),	\
				  w,h,cap->cap_fmt,	\
				  1);
#endif
	if(ret == -1)
		hv_err("save image failed!\n");
	return 0;
}
int do_save_frame(void* capture,int buf_index)
{
	int ret;
	static int index = 0;
	static int interval = 0;
	int tmp_interval = 5;

	capture_handle* cap = (capture_handle*)capture;

	if((cap->show_rate != 0) && (cap->show_rate < cap->cap_fps) && (cap->show_rate > 0))
		tmp_interval = cap->cap_fps/cap->show_rate;
	if(interval-- == 0){
		char name[30];
		int sub_start;
		snprintf(name, sizeof(name), "dev/frame_%d", index);
		if(cap->sensor_type == V4L2_SENSOR_TYPE_RAW){
			sub_start = (unsigned int)(buffers[buf_index].start) + ALIGN_4K(ALIGN_16B(cap->cap_w) * cap->cap_h * 3 >> 1);
			if(cap->sub_rot == 90 || cap->sub_rot == 270){
				//sub_start = (unsigned int)(sub_start) + ALIGN_4K(ALIGN_16B(cap->sub_w) * cap->sub_h * 3 >> 1);
			}
			ret = save_frame_to_file(name,								\
						   (void*)sub_start,					\
						   cap->sub_w,cap->sub_h,cap->cap_fmt,	\
						   1);

			//save_jpeg_frame_by_viraddr("data/camera/bbb.jpg",(void*)sub_start,640,480);
			
		}
		else{
			sub_start = (unsigned int)(buffers[buf_index].start);
			//if(cap->sub_rot == 90 || cap->sub_rot == 270){
			//	sub_start = sub_start + ALIGN_4K(ALIGN_16B(cap->cap_w) * cap->cap_h * 3 >> 1);
			//}
			ret = save_frame_to_file(name,								\
						   (void*)sub_start,					\
						   cap->cap_w,cap->cap_h,cap->cap_fmt,	\
						   1);
		}
		if(ret == -1){
			hv_err("save frame failed!\n");
			return -1;
		}
		set_sync_status((void*)cap,index);
		index++ ;
		if(index > 20) index = 0;
		interval = tmp_interval - 1;
		return 0;
	}
	return 0;
}

