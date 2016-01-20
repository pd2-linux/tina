/*
 * \file        hawkview.h
 * \brief       
 *
 * \version     1.0.0
 * \date        2014-5-22
 * \author      Henrisk <heweihong@allwinnertech.com>
 * 
 * Copyright (c) 2014 Allwinner Technology. All Rights Reserved.
 *
 */

///////////////////////////////////////////////////////////////////////////////
//debug setting
#ifndef __HAWKVIEW_H__
#define __HAWKVIEW_H__
//#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <signal.h>

#include <pthread.h>
#include <time.h>
#include "videodev2.h"

#include "command.h"
#define HAWKVIEW_DBG 1

#if HAWKVIEW_DBG
#define hv_warn(x,arg...) printf("[hawkview_warn]"x,##arg)
#define hv_dbg(x,arg...) printf("[hawkview_dbg]"x,##arg)
#else
#define hv_warn(x,arg...)
#define hv_dbg(x,arg...) 
#endif
#define hv_err(x,arg...) printf("[hawkview_err]xxxx"x,##arg)
#define hv_msg(x,arg...) printf("[hawkview_msg]----"x,##arg)

#ifdef ANDROID_ENV
#include "log.h"
#undef hv_warn
#undef hv_dbg
#undef hv_err
#undef hv_msg
#define hv_err log_err
#define hv_dbg log_dbg
#define hv_msg log_dbg
#define hv_warn log_dbg

#endif
///////////////////////////////////////////////////////////////////////////////
#define ALIGN_4K(x) (((x) + (4095)) & ~(4095))
#define ALIGN_32B(x) (((x) + (31)) & ~(31))
#define ALIGN_16B(x) (((x) + (15)) & ~(15))


typedef enum _capture_status
{
	ON	=	1,
	OFF	=	0,
}capture_status;

typedef enum _video_status
{
	//for video thread
	VIDEO_EXIT			= 100,
	VIDEO_WAIT			= 101,
	VIDEO_START 		= 102,

}video_status;

///////////////////////////////////////////////////////////////////////////////
//display

struct disp_ops
{
	int (*disp_init)(void *);
	int (*disp_set_addr)(int , int , unsigned int *);
	int (*disp_quit)(void *);
	int (*disp_send_command)(void *,command);
};

typedef struct _display
{
	//display
	int x;
	int y;
	int disp_w;
	int disp_h;

	int input_w;
	int input_h;
	
	struct disp_ops *ops;
	
	command state;
}display_handle;

///////////////////////////////////////////////////////////////////////////////
//capture

struct buffer
{
    void   *start;
    size_t length;
    unsigned int phy_addr;
};

struct v4l2_core_ops{
	int (*open_device)(void*);
	int (*set_video_params)(void*);
	int (*req_buffers)(void*);
	int (*stream_on)(void*);
	int (*stream_off)(void*);
};
struct v4l2_platform_ops{
	int (*get_sensor_type)(void*);
	int (*set_flip)(void*);
};

struct _v4l2{
	int video_fd;
	int req_buffer_num;
	struct buffer* buffers;
	struct v4l2_core_ops *core_ops;
	struct v4l2_platform_ops *platform_ops;
}v4l2_handle;

typedef struct _image{
	char path_name[50];
	struct isp_exif_attribute exif;
}image;

struct cap_ops
{
	int (*cap_init)(void*);
	int (*cap_frame)(void*,int (*)(int,int,unsigned int*),pthread_mutex_t*);
	int (*cap_quit)(void*);
	int (*cap_send_command)(void*,command);
};

typedef struct _capture
{
	int video_no;		// /dev/video device
	int subdev_id;		// v4l2 subdevices id

	int sensor_type;	// yuv or raw sensor

	int set_w;			//request target capture size
	int set_h;		

	int cap_w;			//supported capture size
	int cap_h;

	int sub_w;			//sub channel size
	int sub_h;
	int sub_rot;		//sub channel rotation
	
	int cap_fmt;	//capture format
	float cap_fps;	//capture framerate

	image picture;	//take the yuv picture
	image frame;	//capture frame
	
	int show_rate;	//show framerate
	
	struct cap_ops *ops;
	
	command cmd;

	//for status
	capture_status status;
	capture_status save_status;
}capture_handle;



///////////////////////////////////////////////////////////////////////////////
//command
typedef struct _thread
{
	pthread_mutex_t         mutex;
	pthread_cond_t          cond;
	pthread_t       tid;
	void*           status; 
}thread_handle;

typedef struct _hawkview
{

	capture_handle capture;

	display_handle display;

	command cmd;
	video_status status;

	thread_handle cmd_thread;
	thread_handle vid_thread;
}hawkview_handle;

int hawkview_init(hawkview_handle** hv);

void hawkview_start(hawkview_handle* hv);
void hawkview_stop(hawkview_handle* hv);
int  hawkview_release(hawkview_handle* hv);



#endif

