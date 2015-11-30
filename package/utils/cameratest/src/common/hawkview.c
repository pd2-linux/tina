/*
 * \file        hawkview.c
 * \brief       
 *
 * \version     1.0.0
 * \date        2014-5-22
 * \author      Henrisk <heweihong@allwinnertech.com>
 * 
 * Copyright (c) 2014 Allwinner Technology. All Rights Reserved.
 *
 */
#include <pthread.h>

#include "hawkview.h"
#include "path.h"
#define CHECK_CMD_NUM(n,x) 	\
	if (n < x){	\
		hv_err("invalidity cmd num\n");	\
		return -1;	\
	}
#define MAX_CMD_NUM 5
#define CMD_LEN 20

int old_hv_cmd = 0;
int old_hv_status = 0;
struct stat old_stat;

extern int display_register(hawkview_handle* hawkview);
extern int capture_register(hawkview_handle* hawkview);

static int should_start_camera(hawkview_handle* hv)
{
	int ret = 0;
	if(hv->cmd == SET_CAP_SIZE || \
	   hv->cmd == START_STREAMMING ||\
	   hv->cmd == SET_CAP_VIDEO ||\
	   hv->cmd == SET_SUB_ROT ||\
	   hv->cmd == SET_CAP_INFO){
		ret = 1;
	}
	return ret;
}
static void* hawkview_video_thread(void* arg)
{
	int ret;
	hawkview_handle* hv = (hawkview_handle*)arg;
	while(1){
		
		if (old_hv_cmd != hv->cmd){
			hv_dbg("video thread cmd: %d --> %d\n",old_hv_cmd,hv->cmd);
			old_hv_cmd = hv->cmd;
		}
		if (old_hv_status != hv->status){
			hv_dbg("video thread status %d --> %d\n",old_hv_status,hv->status);
			old_hv_status = hv->status;
		}		
		if(hv->status == VIDEO_EXIT) break;//TODO, no test
		
		if(hv->status == VIDEO_WAIT) {
			pthread_mutex_lock(&hv->vid_thread.mutex);
			if(should_start_camera(hv)){
				hv_dbg("reset video capture\n");
				ret = hv->capture.ops->cap_init((void*)&hv->capture);
				if(ret == -1) return (void*)-1;
				hv->capture.ops->cap_send_command((void*)(&hv->capture),START_STREAMMING);
				hv->status = VIDEO_START;
			}
			else{
				pthread_cond_wait(&hv->vid_thread.cond, &hv->vid_thread.mutex);
			}
			
			pthread_mutex_unlock(&hv->vid_thread.mutex);
			continue;

		}
		if(hv->status == VIDEO_START) {
			pthread_mutex_lock(&hv->vid_thread.mutex);
			if(hv->cmd == STOP_STREAMMING)				
				hv->capture.ops->cap_send_command((void*)(&hv->capture),STOP_STREAMMING);
			pthread_mutex_unlock(&hv->vid_thread.mutex);
			if(hv->capture.ops){
				ret = hv->capture.ops->cap_frame((void*)(&hv->capture),NULL,&hv->vid_thread.mutex);
			}
			if(ret == 0) {
				//pthread_mutex_unlock(&hv->vid_thread.mutex);
				continue;
			}
			
			if(ret == 2) {
				hv_dbg("video wait\n");
				pthread_mutex_lock(&hv->vid_thread.mutex);
				hv->status = VIDEO_WAIT;
				if(!should_start_camera(hv)){
					pthread_cond_wait(&hv->vid_thread.cond, &hv->vid_thread.mutex);
				}
				
				pthread_mutex_unlock(&hv->vid_thread.mutex);
				continue;
			}

			if(ret == -1)
				return (void*)-1;
		}
		
	}

	if(hv->capture.ops->cap_quit)
		hv->capture.ops->cap_quit((void*)(&hv->capture));
		
	return (void*)0;

}


int fetch_sub_cmd(const char* buf,char** cmd,int* cmd_num,int lenght)
{
		int i = 0,j = 0,n = 0;

		while(buf[i] != '#'){	//the sub cmd end by '#'
			while(buf[i] != 'x' && buf[i] != ':' && buf[i] != '#')	{
				*((char*)cmd + n*lenght + j++) = buf[i++];
				if(j > lenght) {
					hv_err("sub cmd over long\n");
					*cmd_num = n + 1;
					return -1;
				}
			}
			*((char*)cmd + n*lenght + j++) = '\0';
			n++;
			j = 0;
			if(buf[i] != '#'){
				i++;
			}
			if(n > *cmd_num){
				hv_err("the max cmd num is %d\n",*cmd_num);
				return -1;
				
			}
		}
		*cmd_num = n;
		return 0;
}
int fetch_cmd(hawkview_handle* hv)
{
	int ret = 0;
	int i;
	FILE* fp = NULL;
	char buf[200];
	char cmd[MAX_CMD_NUM][CMD_LEN];
	int n = MAX_CMD_NUM;
	
	struct stat cmd_stat;
	//command file
	sprintf(buf,"%s/command",PATH);
	ret = lstat(buf,&cmd_stat);
	if(ret == -1){
		//hv_err("can't lstat /data/camera/command,%s\n",strerror(errno));
		return ret;
	}
	
	if(cmd_stat.st_ctime == old_stat.st_ctime) return 0;
	
	old_stat.st_ctime = cmd_stat.st_ctime;
	fp = fopen(buf,"rwb+");
	if(fp){
		//todo flock;
		//if (flock(fp->_fileno, LOCK_EX) != 0){ // file lock_exclusive
		//	hv_dbg("file lock by others\n");
		//}
		memset(buf,0,sizeof(buf));
		memset(cmd,0,sizeof(cmd));
		ret = fread(buf,50,50,fp);
		fclose(fp);
		//flock(fp->_fileno, LOCK_UN); //unlock

	}
	hv_dbg("read cmd %s\n",buf);
	ret = fetch_sub_cmd(buf,(char**)cmd,&n,CMD_LEN);
	for(i = 0;i < n;i++)
		hv_dbg("cmd %d: %s\n",i,cmd[i]);
	
	ret = atoi(cmd[0]);
	if(ret == SET_CAP_SIZE){
		//eg: command string "148:1280x720#"
		CHECK_CMD_NUM(n,2);
		hv->capture.set_w = atoi(cmd[1]);
		hv->capture.set_h = atoi(cmd[2]);
		hv_dbg("set size: %d x %d\n",hv->capture.set_w,hv->capture.set_h);		
	}

	if(ret == SET_CAP_VIDEO){
		//eg: command string "147:0:1#"   video:0,s_input:1
		CHECK_CMD_NUM(n,2);
		hv->capture.video_no = atoi(cmd[1]);
		hv->capture.subdev_id = atoi(cmd[2]);
	}
	if(ret == SET_CAP_INFO){
		CHECK_CMD_NUM(n,5);
		hv->capture.video_no = atoi(cmd[1]);
		hv->capture.subdev_id = atoi(cmd[2]);
		hv->capture.set_w = atoi(cmd[3]);
		hv->capture.set_h = atoi(cmd[4]);

	}
	if(ret == SAVE_FRAME){
		//CHECK_CMD_NUM(n,2);
		hv->capture.show_rate = atoi(cmd[1]);
	}
	if(ret == SAVE_IMAGE){
		//CHECK_CMD_NUM(n,2);
		strcpy(hv->capture.picture.path_name,cmd[1]);
	}
	if(ret == SET_SUB_ROT){
		CHECK_CMD_NUM(n,2);
		hv->capture.sub_rot = atoi(cmd[1]);
	}
	return 	ret;

		
	
	
}
void send_command(hawkview_handle* hv,int cmd)
{
	
	if (cmd == SET_SUB_ROT){				//command:145
		hv->capture.ops->cap_send_command((void*)(&hv->capture),STOP_STREAMMING);
		hv->cmd = cmd;

	}else if (cmd == SET_CAP_INFO){			//command:146
		hv->capture.ops->cap_send_command((void*)(&hv->capture),STOP_STREAMMING);
		hv->cmd = cmd;

	}else if (cmd == SET_CAP_VIDEO){		//command:147
		hv->capture.ops->cap_send_command((void*)(&hv->capture),STOP_STREAMMING);
		hv->cmd = cmd;

	}else if (cmd == SET_CAP_SIZE){			//command:148
		hv->capture.ops->cap_send_command((void*)(&hv->capture),STOP_STREAMMING);
		hv->cmd = cmd;

	}else if (cmd == SAVE_IMAGE){			//command:149
		hv->capture.ops->cap_send_command((void*)(&hv->capture),SAVE_IMAGE);

	}else if (cmd == SAVE_FRAME){			//command:150
		hv->capture.ops->cap_send_command((void*)(&hv->capture),SAVE_FRAME);
			
	}else if (cmd == STOP_SAVE_FRAME){		//command:151
		hv->capture.ops->cap_send_command((void*)(&hv->capture),STOP_SAVE_FRAME);

	}else if (cmd == STOP_STREAMMING){		//command:160
		hv->cmd = STOP_STREAMMING;
		
	}else if (cmd == START_STREAMMING){		//command:161
		hv->cmd = START_STREAMMING;

	}else if (cmd == FULL_SCREEN){			//command:200
		//hv->display.ops->disp_send_command((void*)(&hv->display),FULL_SCREEN);

	}else if (cmd == FULL_CAPTURE){			//command:201
		//hv->display.ops->disp_send_command((void*)(&hv->display),FULL_CAPTURE);
	}

    return;
}

static void* hawkview_command_thread(void* arg)
{
	int ret;
	hawkview_handle* hv = (hawkview_handle*)arg;
	while(1){
		//fetch the message for command file: data/camera/command
                usleep(200000);
		ret = fetch_cmd(hv);
		if(ret <= 0)
			continue;

		if (ret == COMMAND_EXIT){
			hv_dbg("hawkview command thread exit!");
			break;
		}
		else if (ret == COMMAND_WAIT){
			//TODO
		}
		
		pthread_mutex_lock(&hv->vid_thread.mutex);
		hv_dbg("send command %d\n",ret);
		send_command(hv,ret);
		pthread_mutex_unlock(&hv->vid_thread.mutex);
		pthread_cond_signal(&hv->vid_thread.cond);
	}
	return 0;
}

int start_video_thread(hawkview_handle* hv)
{
	int ret;
	hv->vid_thread.tid = 0;
	ret = pthread_create(&hv->vid_thread.tid, NULL, hawkview_video_thread, (void *)hv);
	hv_dbg("video pthread_create ret:%d\n",ret);
    if ( ret == -1) {
       	hv_err("camera: can't create video thread(%s)\n", strerror(errno));
    	return -1;
	}
	return ret;
}

int start_command_thread(hawkview_handle* hv)
{
	int ret;
	hv->cmd_thread.tid = 0;
	ret = pthread_create(&hv->cmd_thread.tid, NULL, hawkview_command_thread, (void *)hv);
	hv_dbg("command pthread_create ret:%d\n",ret);
    if ( ret == -1) {
       	hv_err("camera: can't create command thread(%s)\n", strerror(errno));
    	return -1;
	}
	return ret;
	
}
static int init_defualt_parameters(hawkview_handle* hv)
{	
	hv->capture.set_w = 1280;
	hv->capture.set_h = 720;
	hv->capture.video_no = 1;
	hv->capture.subdev_id = 0;
	hv->capture.cap_fps = 30;
	hv->capture.cap_fmt = V4L2_PIX_FMT_NV12;
	hv->capture.sub_w = 640;
	hv->capture.sub_h = 480;
	hv->display.input_w = 640;
	hv->display.input_h = 480;

	return 0;

}
int hawkview_init(hawkview_handle** hv)
{
	int ret;
	hawkview_handle *hawkview;

	memset(&old_stat,0,sizeof(struct stat));

	hawkview = malloc(sizeof(hawkview_handle));
	if(hawkview == NULL){
		hv_err("malloc hawkview faided!\n");
		return -1;
	}		
	memset(hawkview,0,sizeof(hawkview_handle));
	*hv = hawkview;

	init_defualt_parameters(hawkview);
	
	hv_dbg("hawkview_init set_w %d\n",hawkview->capture.set_w);
	
	//ret = display_register(hawkview);
	//if(ret == -1){
	//	hv_err("display_register failed\n");
	//	return -1;
	//}

	ret = capture_register(hawkview);
	if(ret == -1){
		hv_err("capture_register failed\n");
		return -1;
	}
	
	if(hawkview->display.ops){
		ret = hawkview->display.ops->disp_init((void*)&hawkview->display);
		if(ret == -1) {
			hv_err("display init fail!\n");
			return -1;
		}else hv_msg("display init sucessfully\n");
	}
	hv_dbg("hawkview_init 2 \n");
	pthread_mutex_init(&hawkview->cmd_thread.mutex, NULL);
	pthread_cond_init(&hawkview->cmd_thread.cond, NULL);

	pthread_mutex_init(&hawkview->vid_thread.mutex, NULL);
	pthread_cond_init(&hawkview->vid_thread.cond, NULL);

	hawkview->status = VIDEO_WAIT;
	
	return 0;
}

void hawkview_start(hawkview_handle* hv)
{
	start_video_thread(hv);
	start_command_thread(hv);
	
	pthread_join(hv->cmd_thread.tid,&hv->cmd_thread.status);
	pthread_join(hv->vid_thread.tid,&hv->vid_thread.status);
}

void hawkview_stop(hawkview_handle* hv)
{

}

int hawkview_release(hawkview_handle* hv)
{
	//TODO
	//pthread_mutex_destroy(&video_mutex);
	//pthread_cond_destroy(&video_cond);
        return 0;
}