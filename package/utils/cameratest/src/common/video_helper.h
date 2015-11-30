#ifndef __VIDEO_HELPER_H__
#define __VIDEO_HELPER_H__
#include "hawkview.h"


int is_x_sec(int sec,long long secs,long long usecs);

float get_framerate(long long secs,long long usecs);

int write_file(char* file_path,char* string,int lenght );

int make_exif_info(char* exif_str,char* name, struct isp_exif_attribute *exif,int w,int h);

int set_exif_info(void* capture);

int set_cap_info(void* capture);

int set_sync_status(void* capture,int index);

int save_frame_to_file(void* str,void* start,int w,int h,int format,int is_one_frame);

int get_disp_addr(void* capture,unsigned int origin,unsigned int* addr,int* w,int* h);

int do_save_image(void* capture,int buf_index);

int do_save_frame(void* capture,int buf_index);

#endif
