#include "hawkview.h"
#include "video_helper.h"

#define DELIVER_FRAMES_RATE 5


struct buffer *buffers = NULL;
static int nbuffers = 0;
static int videofh = 0;
static int req_frame_num = 10;

int old_status = -1;
int old_vi_cmd = -1;

int get_framerate_status = 4;

static int setVflipHflip(void* capture)
{
	int ret = -1;
	struct v4l2_control ctrl;
	capture_handle* cap = (capture_handle*)capture;
	hv_msg("set vflip and hflip\n");
	
	if(cap->sensor_type == 0){
		ctrl.id = V4L2_CID_VFLIP;
		ctrl.value = 1;
		ret = ioctl(videofh, VIDIOC_S_CTRL, &ctrl);
		if (ret != 0)
			hv_err("set vflip fail\n");

		ctrl.id = V4L2_CID_HFLIP;
		ctrl.value = 1;
		ret = ioctl(videofh, VIDIOC_S_CTRL, &ctrl);
		if (ret != 0)
			hv_err("set hflip fail\n");
	}else{
		ctrl.id = V4L2_CID_VFLIP_THUMB;
		ctrl.value = 1;
		ret = ioctl(videofh, VIDIOC_S_CTRL, &ctrl);
		if (ret != 0)
			hv_err("set vflip fail\n");

		ctrl.id = V4L2_CID_HFLIP_THUMB;
		ctrl.value = 1;
		ret = ioctl(videofh, VIDIOC_S_CTRL, &ctrl);
		if (ret != 0)
			hv_err("set hflip fail\n");

	}	
	return ret;
}
static int openDevice(void* capture)
{
	char dev_name[32];
	capture_handle* cap = (capture_handle*)capture;
	snprintf(dev_name, sizeof(dev_name), "/dev/video%d", cap->video_no);

	hv_msg("open %s\n", dev_name);
	if ((videofh = open(dev_name, O_RDWR,0)) < 0) {
		hv_err("can't open %s(%s)\n", dev_name, strerror(errno));
		return -1;
	}

	fcntl(videofh, F_SETFD, FD_CLOEXEC);
	return 0;
	
}
static int setVideoParams(void* capture)
{
	capture_handle* cap = (capture_handle*)capture;
	
	/* set input input index */
	struct v4l2_input inp;
	inp.index = cap->subdev_id;
	inp.type = V4L2_INPUT_TYPE_CAMERA;
	if (ioctl(videofh, VIDIOC_S_INPUT, &inp) == -1) {
		hv_err("VIDIOC_S_INPUT failed! s_input: %d\n",inp.index);
		inp.index = (inp.index == 1)?0:1;
		if (ioctl(videofh, VIDIOC_S_INPUT, &inp) == -1){
			hv_err("VIDIOC_S_INPUT failed! s_input: %d\n",inp.index);
			return -1;
		}
	}

	struct v4l2_streamparm parms;
	parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//parms.parm.capture.capturemode = V4L2_MODE_PREVIEW;
	parms.parm.capture.timeperframe.numerator = 1;
	parms.parm.capture.timeperframe.denominator = cap->cap_fps;
	if (ioctl(videofh, VIDIOC_S_PARM, &parms) == -1) {
		hv_err("VIDIOC_S_PARM failed!\n");
		return -1;
	}

	/* set image format */
	struct v4l2_format fmt;
	struct v4l2_pix_format sub_fmt;
	memset(&fmt, 0, sizeof(struct v4l2_format));
	memset(&sub_fmt, 0, sizeof(struct v4l2_pix_format));
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = cap->set_w;
	fmt.fmt.pix.height      = cap->set_h;
	fmt.fmt.pix.pixelformat = cap->cap_fmt;
	fmt.fmt.pix.field       = V4L2_FIELD_NONE;

	if(cap->sensor_type == V4L2_SENSOR_TYPE_RAW){
		fmt.fmt.pix.subchannel = &sub_fmt;
		fmt.fmt.pix.subchannel->width = cap->sub_w;
		fmt.fmt.pix.subchannel->height = cap->sub_h;
		fmt.fmt.pix.subchannel->pixelformat = cap->cap_fmt;
		fmt.fmt.pix.subchannel->field = V4L2_FIELD_NONE;
		if(cap->sub_rot != 0)
			hv_msg("subchannel rot_angle: %d\n",cap->sub_rot);
			fmt.fmt.pix.subchannel->rot_angle = cap->sub_rot;		
	}else{
		if(cap->sub_rot != 0){			
			hv_msg("rot_angle: %d\n",cap->sub_rot);
			fmt.fmt.pix.rot_angle = cap->sub_rot;
		}
	}
	if (ioctl(videofh, VIDIOC_S_FMT, &fmt)<0) {
		hv_err("VIDIOC_S_FMT failed!\n");
		return -1;
	}
	hv_msg("the tried size is %dx%d,the supported size is %dx%d!\n",\
				cap->set_w,		\
				cap->set_h, 	\
				fmt.fmt.pix.width,	\
				fmt.fmt.pix.height);

	cap->cap_w = fmt.fmt.pix.width;
	cap->cap_h = fmt.fmt.pix.height;
	
	if(cap->sensor_type == V4L2_SENSOR_TYPE_RAW){
		hv_msg("the subchannel size is %dx%d\n",fmt.fmt.pix.subchannel->width,fmt.fmt.pix.subchannel->height);
		cap->sub_w = fmt.fmt.pix.subchannel->width;
		cap->sub_h = fmt.fmt.pix.subchannel->height;
	}
	return 0;
	
}
static int reqBuffers(void* capture)
{
	int i;
	struct v4l2_requestbuffers req;
	capture_handle* cap = (capture_handle*)capture;
	struct v4l2_buffer buf;
	/* request buffer */
	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count  = req_frame_num;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if(ioctl(videofh, VIDIOC_REQBUFS, &req)<0){
		hv_err("VIDIOC_REQBUFS failed\n");
		return -1;
	}

	buffers = calloc(req.count, sizeof(struct buffer));
	for (nbuffers = 0; nbuffers < req.count; nbuffers++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = nbuffers;

		if (ioctl(videofh, VIDIOC_QUERYBUF, &buf) == -1) {
			hv_err("VIDIOC_QUERYBUF error\n");
			return -2;	//goto release buffer
		}

		buffers[nbuffers].start  = mmap(NULL, buf.length, 
										PROT_READ | PROT_WRITE, \
										MAP_SHARED, videofh, \
										buf.m.offset);
		buffers[nbuffers].length = buf.length;

		if (buffers[nbuffers].start == MAP_FAILED) {
			hv_err("mmap failed\n");
			for(i = 0;i < nbuffers;i++){
				hv_dbg("munmap buffer index: %d,mem: %x",i,(int)buffers[nbuffers].start);
				munmap(buffers[i].start, buffers[i].length);
			}
			return -2;	//goto release buffer
		}
		hv_dbg("map buffer index: %d, mem: %x, len: %x, offset: %x\n",
				nbuffers,(int)buffers[nbuffers].start,buf.length,buf.m.offset);
	}
	for(nbuffers = 0; nbuffers < req.count; nbuffers++){
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = nbuffers;
		if (ioctl(videofh, VIDIOC_QBUF, &buf) == -1){ 
			hv_err("VIDIOC_QBUF error\n");
			return -3;//goto umap
		}
	}
	return 0;
}
int getExifInfo(struct isp_exif_attribute *exif)
{
	int ret = -1;
	if (videofh == NULL)
	{
		return 0xFF000000;
	}
	ret = ioctl(videofh, VIDIOC_ISP_EXIF_REQ, exif);
	return ret;
}

static int getSensorType(void* capture)
{
	int ret = -1;
	struct v4l2_control ctrl;
	struct v4l2_queryctrl qc_ctrl;

	capture_handle* cap = (capture_handle*)capture;

	if (videofh == NULL)
	{
		return 0xFF000000;
	}

	ctrl.id = V4L2_CID_SENSOR_TYPE;
	qc_ctrl.id = V4L2_CID_SENSOR_TYPE;

	if (-1 == ioctl (videofh, VIDIOC_QUERYCTRL, &qc_ctrl))
	{
		hv_err("query sensor type ctrl failed");
		return -1;
	}
	ret = ioctl(videofh, VIDIOC_G_CTRL, &ctrl);
	cap->sensor_type = ctrl.value;
	return ret;
}

static int capture_init(void* capture)
{

	int ret;
	int i;
	
	capture_handle* cap = (capture_handle*)capture;
	cap->save_status = OFF;
	cap->status = OFF;
	cap->cmd = COMMAND_UNUSED;
	get_framerate_status = 4;

	ret = openDevice(capture);
	if(ret == -1)
		goto open_err;

	getSensorType(capture);
	hv_msg("get sensor type: %d\n",cap->sensor_type);
	sleep(1);
	ret = setVideoParams(capture);
	if(ret == -1)
		goto err;

	if(cap->sub_rot == 180)
		setVflipHflip(capture);
	ret = reqBuffers(capture);
	if(ret == -1)
		goto err;
	else if(ret == -2)
		goto buffer_rel;
	else if(ret == -3)
		goto unmap;

	return 0;

unmap:
	for (i = 0; i < nbuffers; i++) {
		munmap(buffers[i].start, buffers[i].length);
	}
buffer_rel:
	free(buffers);	
err:
	close(videofh);
open_err:
	return -1;
}

static int capture_frame(void* capture,int (*set_disp_addr)(int,int,unsigned int*),pthread_mutex_t* mutex)
{
	capture_handle* cap = (capture_handle*)capture;
	int ret;
	int i;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;	

	fd_set fds;
	struct timeval tv;
	pthread_mutex_lock(mutex);
	//used for cammand and status debug
	if (old_vi_cmd != cap->cmd){
		hv_dbg("capture frame command %d --> %d\n",old_vi_cmd,cap->cmd);
		old_vi_cmd = (int)cap->cmd;
	}
	if(old_status != cap->status){
		hv_dbg("capture frame status  %d --> %d\n",old_status,cap->status);
		old_status = cap->status;
	}

	if(cap->status == OFF && cap->cmd == START_STREAMMING){
		hv_dbg("capture start streaming\n");
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(videofh, VIDIOC_STREAMON, &type) == -1) {
			hv_err("VIDIOC_STREAMON error! %s\n",strerror(errno));
			goto quit;
		}
		cap->status = ON;
		cap->cmd = COMMAND_UNUSED;
		pthread_mutex_unlock(mutex);
		return 0;
	}
	
	if(cap->status == ON && cap->cmd == STOP_STREAMMING){
		hv_dbg("capture stop streaming\n");
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(-1 == ioctl(videofh, VIDIOC_STREAMOFF, &type)){
			hv_err("VIDIOC_STREAMOFF error! %s\n",strerror(errno));
			goto quit;
		}
		cap->status = OFF;
		cap->cmd = COMMAND_UNUSED;
		capture_quit(capture);
		pthread_mutex_unlock(mutex);
		return 2;
	}

	if(cap->status == OFF) {
		pthread_mutex_unlock(mutex);
		return 0;
	}
	FD_ZERO(&fds);
	FD_SET(videofh, &fds);

	tv.tv_sec  = 2;
	tv.tv_usec = 0;
	pthread_mutex_unlock(mutex);
	ret = select(videofh + 1, &fds, NULL, NULL, &tv);
	pthread_mutex_lock(mutex);
	//hv_dbg("select video ret: %d\n",ret);
	if (ret == -1) {
		if (errno == EINTR) {
			return 0;
		}
		hv_err("select error\n");
		goto stream_off;
	}
	else if (ret == 0) {
		hv_err("select timeout\n");
		return 0;
	}

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(videofh, VIDIOC_DQBUF, &buf);
	if (ret == -1) {
		hv_err("VIDIOC_DQBUF failed!\n");
		goto stream_off;
	}


	
	float framerate;
	framerate = get_framerate((long long)(buf.timestamp.tv_sec),(long long)(buf.timestamp.tv_usec));
	if(framerate > 1.0){
		cap->cap_fps = framerate;
		//hv_dbg("framerate: %0.2ffps\n",cap->cap_fps);
	}

	//sync capture info perp x second
#define M_SECOND 200	
	if(is_x_msec(M_SECOND,(long long)(buf.timestamp.tv_sec),(long long)(buf.timestamp.tv_usec))){
		getExifInfo(&(cap->frame.exif));
		set_cap_info((void*)cap);
	}

	if(cap->cmd == STOP_SAVE_FRAME && cap->save_status == ON)
		cap->save_status = OFF;

	//save frame , the frame will be get by PC Tool to preview on PC screen
	//frame format: /dev/frame_x  (x:0~21)
	if(cap->cmd == SAVE_FRAME || cap->save_status == ON ) {
		
		if(cap->cmd == SAVE_FRAME){
			cap->save_status = ON;
			cap->cmd = COMMAND_UNUSED;
		}
		ret = do_save_frame(capture,buf.index);
	}

	//take yuv image,it will save the target frame exif info in the same time
	//image name: 		xxxx (set by usered through command)
	//exif info name: 	xxxx.exif
	if(cap->cmd == SAVE_IMAGE ) {
		ret = 0;//getExifInfo(&(cap->picture.exif));
		//get target frame exif info successfully then save the target image
		//if get the exif info fail,it will try next frame
		if(ret == 0){
			buffers[buf.index].phy_addr = buf.m.offset - 0x20000000;
			hv_dbg("index: %d buffers[buf.index].start = %p\n",buf.index,buffers[buf.index].start);
			//do_save_image(capture,buf.index);
			do_save_sub_image(capture,buf.index);
			cap->cmd = COMMAND_UNUSED;
		}
	}

	//get display addr
	int w,h;
	unsigned int addr;
	get_disp_addr(capture, buf.m.offset,&addr,&w,&h);

	// set disp buffer
	if (set_disp_addr){
		set_disp_addr(w,h,&addr);
	}
	
	ret = ioctl(videofh, VIDIOC_QBUF, &buf);
	if (ret == -1) {
		hv_err("VIDIOC_DQBUF failed!\n");
		goto stream_off;
	}

	pthread_mutex_unlock(mutex);
	return 0;
	
stream_off:
	hv_err("err stream off\n");
	ioctl(videofh, VIDIOC_STREAMOFF, &type);
quit:
	capture_quit(capture);
	pthread_mutex_unlock(mutex);
	return -1;

	
}

int capture_quit(void *capture)
{
	int i;
	enum v4l2_buf_type type;
	hv_msg("capture quit!\n");
	for (i = 0; i < nbuffers; i++) {
		hv_dbg("ummap index: %d, mem: %x, len: %x\n",
				i,(int)buffers[i].start,buffers[i].length);
		munmap(buffers[i].start, buffers[i].length);
	}
	free(buffers);
	close(videofh);
	return 0;
}

int capture_command(void* capture,command state)
{
	capture_handle* cap = (capture_handle*)capture;
	//todo: add metux
	cap->cmd = state;
	return 0;
}


static struct cap_ops ops = {
	.cap_init  = capture_init,
	.cap_frame = capture_frame,
	.cap_quit  = capture_quit,
	.cap_send_command = capture_command,

};
static const capture_handle sun9iw1p1_cap = {
	.ops = &ops,
};

int capture_register(hawkview_handle* hawkview)
{
	if (hawkview == NULL){
		hv_err("hawkview handle is NULL,sunxi9iw1p1 capture register fail\n");
		return -1;
	}
	hawkview->capture.ops = &ops;
	hv_msg("sunxi9iw1p1 capture register sucessfully!\n");
	return 0;
}

