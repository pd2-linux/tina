
//#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_DETAIL
//#define LOG_TAG "layerControl_android_newDisplay"
#include "config.h"
#include "player_i.h"

#if CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_2
#include <gui/ISurfaceTexture.h>
#elif CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_4_4
#include <gui/Surface.h>
#else
    #error "invalid configuration of os version."
#endif

#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include "layerControl.h"
#include "log.h"
#include "memoryAdapter.h"
#include <hardware/hwcomposer.h>

#if(CONFIG_CHIP != OPTION_CHIP_1651)
#include <hardware/hal_public.h>
#endif
#include <linux/ion.h>
#include <ion/ion.h>

#if (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0 && CONFIG_CHIP == OPTION_CHIP_1667)
#include "gralloc_priv.h"
#endif

#define GPU_BUFFER_NUM 32

#if CONFIG_CHIP == OPTION_CHIP_1639
#define PHY_OFFSET 0x20000000
#else
#define PHY_OFFSET 0x40000000
#endif

#if 0
/* NUM_OF_PICTURES_KEEP_IN_LIST has defined in config.h */
#if(CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX)
/* 3 allow drop frame in SurfaceFlinger */
#define NUM_OF_PICTURES_KEEP_IN_LIST  4
#else
#define NUM_OF_PICTURES_KEEP_IN_LIST  2
#endif
#endif // #if 0

/* +1 allows queue after SetGpuBufferToDecoder */
#define NUM_OF_PICTURES_KEEP_IN_NODE (NUM_OF_PICTURES_KEEP_IN_LIST+1)

typedef struct VPictureNode_t VPictureNode;
struct VPictureNode_t
{
    VideoPicture* pPicture;
    int           bUsed;
};

typedef struct GpuBufferInfoS
{
    ANativeWindowBuffer* pWindowBuf;

#if (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0)
	ion_user_handle_t handle_ion;
#else
	struct ion_handle *handle_ion;
#endif

    char* pBufPhyAddr;
    char* pBufVirAddr;
    int   nUsedFlag;
    int   nDequeueFlag;
}GpuBufferInfoT;

typedef struct LayerCtrlContext
{
    ANativeWindow*       pNativeWindow;
    enum EPIXELFORMAT    eReceivePixelFormat;
    int                  nWidth;
    int                  nHeight;
    int                  nLeftOff;
    int                  nTopOff;
    int                  nDisplayWidth;
    int                  nDisplayHeight;
    LayerCtlCallback       callback;
    void*                pUserData;
    enum EPICTURE3DMODE  ePicture3DMode;
    enum EDISPLAY3DMODE  eDisplay3DMode;
    int                  bLayerInitialized;
    int                  bLayerShowed;
    int                  bProtectFlag;
    
    //* use when render derect to hardware layer.
    VPictureNode         picNodes[NUM_OF_PICTURES_KEEP_IN_NODE];

    GpuBufferInfoT       mGpuBufferInfo[GPU_BUFFER_NUM];
    int                  nGpuBufferCount;
    int                  ionFd;
    int                  b4KAlignFlag;
    int                  bHoldLastPictureFlag;
    int                  bVideoWithTwoStreamFlag;
    int                  bIsSoftDecoderFlag;
    unsigned int         nUsage;
}LayerCtrlContext;

//* this function just for 3D case.
//* just init 32-line buffer to black color.
//* (when the two stream display to 2D, the 32-line buffer will cause "Green Screen" if not init, 
//*  as buffer have make 32-align)
//* if init the whole buffer, it would take too much time.
int initPartialGpuBuffer(char* pDataBuf, ANativeWindowBuffer* pWindowBuf, LayerCtrlContext* lc)
{
    logv("initGpuBuffer, stride = %d, height = %d, ",pWindowBuf->stride,pWindowBuf->height);

    if(lc->eReceivePixelFormat == PIXEL_FORMAT_NV21)
    {
    	//* Y1
    	int nRealHeight = pWindowBuf->height/2;
    	int nInitHeight = 32;
    	int nSkipLen = pWindowBuf->stride*(nRealHeight - nInitHeight);
    	int nCpyLenY = pWindowBuf->stride*nInitHeight;
    	memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);
    	//* Y2
    	nSkipLen += pWindowBuf->stride*nRealHeight;
    	memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);

    	//*UV1
    	nSkipLen += nCpyLenY;
    	nSkipLen += (pWindowBuf->stride/2)*(nRealHeight/2 - nInitHeight/2);
    	int nCpyLenUV = (pWindowBuf->stride/2)*(nInitHeight/2);
    	memset(pDataBuf+nSkipLen, 0x80, nCpyLenUV);
    	//*UV2
    	nSkipLen += (pWindowBuf->stride/2)*(nRealHeight/2);
    	memset(pDataBuf+nSkipLen, 0x80, nCpyLenUV);
    }
    else
    {
        loge("the pixelFormat is not support when initPartialGpuBuffer, pixelFormat = %d",
              lc->eReceivePixelFormat);
        return -1;
    }

	return 0;
}

//* copy from ACodec.cpp
static int pushBlankBuffersToNativeWindow(LayerCtrlContext* lc) 
{
    logd("pushBlankBuffersToNativeWindow: pNativeWindow = %p",lc->pNativeWindow);

    if(lc->pNativeWindow == NULL)
    {
        logw(" the nativeWindow is null when call pushBlankBuffersToNativeWindow");
        return 0;
    }
    status_t err = NO_ERROR;
    ANativeWindowBuffer* pWindowBuffer = NULL;
    int numBufs = 0;
    int minUndequeuedBufs = 0;
    ANativeWindowBuffer **pArrBuffer = NULL;

    // We need to reconnect to the ANativeWindow as a CPU client to ensure that
    // no frames get dropped by SurfaceFlinger assuming that these are video
    // frames.
    err = native_window_api_disconnect(lc->pNativeWindow,NATIVE_WINDOW_API_MEDIA);
    if (err != NO_ERROR) {
        loge("error pushing blank frames: api_disconnect failed: %s (%d)",
                strerror(-err), -err);
        return err;
    }

    err = native_window_api_connect(lc->pNativeWindow,NATIVE_WINDOW_API_CPU);
    if (err != NO_ERROR) {
        loge("error pushing blank frames: api_connect failed: %s (%d)",
                strerror(-err), -err);
        return err;
    }

    err = native_window_set_buffers_geometry(lc->pNativeWindow, 1, 1,
            HAL_PIXEL_FORMAT_RGBX_8888);
    if (err != NO_ERROR) {
        loge("error pushing blank frames: set_buffers_geometry failed: %s (%d)",
                strerror(-err), -err);
        goto error;
    }

    err = native_window_set_scaling_mode(lc->pNativeWindow,
                NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    if (err != NO_ERROR) {
        loge("error pushing blank_frames: set_scaling_mode failed: %s (%d)",
              strerror(-err), -err);
        goto error;
    }

    err = native_window_set_usage(lc->pNativeWindow,
            GRALLOC_USAGE_SW_WRITE_OFTEN);
    if (err != NO_ERROR) {
        loge("error pushing blank frames: set_usage failed: %s (%d)",
                strerror(-err), -err);
        goto error;
    }

    err = lc->pNativeWindow->query(lc->pNativeWindow,
            NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBufs);
    if (err != NO_ERROR) {
        loge("error pushing blank frames: MIN_UNDEQUEUED_BUFFERS query "
                "failed: %s (%d)", strerror(-err), -err);
        goto error;
    }

    numBufs = minUndequeuedBufs + 1;
    if (numBufs < 3)
        numBufs = 3;
    err = native_window_set_buffer_count(lc->pNativeWindow, numBufs);
    if (err != NO_ERROR) {
        loge("error pushing blank frames: set_buffer_count failed: %s (%d)",
                strerror(-err), -err);
        goto error;
    }

    // We  push numBufs + 1 buffers to ensure that we've drawn into the same
    // buffer twice.  This should guarantee that the buffer has been displayed
    // on the screen and then been replaced, so an previous video frames are
    // guaranteed NOT to be currently displayed.

    logd("numBufs=%d", numBufs);
    //* we just push numBufs.If push numBus+1,it will be problem in suspension window
    for (int i = 0; i < numBufs; i++) {
        int fenceFd = -1;
        err = native_window_dequeue_buffer_and_wait(lc->pNativeWindow, &pWindowBuffer);
        if (err != NO_ERROR) {
            loge("error pushing blank frames: dequeueBuffer failed: %s (%d)",
                    strerror(-err), -err);
            goto error;
        }

        sp<GraphicBuffer> buf(new GraphicBuffer(pWindowBuffer, false));

        // Fill the buffer with the a 1x1 checkerboard pattern ;)
        uint32_t* img = NULL;
        err = buf->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&img));
        if (err != NO_ERROR) {
            loge("error pushing blank frames: lock failed: %s (%d)",
                    strerror(-err), -err);
            goto error;
        }

        *img = 0;

        err = buf->unlock();
        if (err != NO_ERROR) {
            loge("error pushing blank frames: unlock failed: %s (%d)",
                    strerror(-err), -err);
            goto error;
        }

        err = lc->pNativeWindow->queueBuffer(lc->pNativeWindow,
                buf->getNativeBuffer(), -1);
        if (err != NO_ERROR) {
            loge("error pushing blank frames: queueBuffer failed: %s (%d)",
                    strerror(-err), -err);
            goto error;
        }

        pWindowBuffer = NULL;
    }

    pArrBuffer = (ANativeWindowBuffer **)malloc((numBufs)*sizeof(ANativeWindowBuffer*));
    for (int i = 0; i < numBufs-1; ++i) {
        err = native_window_dequeue_buffer_and_wait(lc->pNativeWindow, &pArrBuffer[i]);
        if (err != NO_ERROR) {
            loge("error pushing blank frames: dequeueBuffer failed: %s (%d)",
                        strerror(-err), -err);
            goto error;
        }
    }
    for (int i = 0; i < numBufs-1; ++i) {
        lc->pNativeWindow->cancelBuffer(lc->pNativeWindow, pArrBuffer[i], -1);
    }
    free(pArrBuffer);
    pArrBuffer = NULL;

error:

    if (err != NO_ERROR) {
        // Clean up after an error.
        if (pWindowBuffer != NULL) {
            lc->pNativeWindow->cancelBuffer(lc->pNativeWindow, pWindowBuffer, -1);
        }

        if (pArrBuffer) {
            free(pArrBuffer);
        }

        native_window_api_disconnect(lc->pNativeWindow,
                NATIVE_WINDOW_API_CPU);
        native_window_api_connect(lc->pNativeWindow,
                NATIVE_WINDOW_API_MEDIA);

        return err;
    } else {
        // Clean up after success.
        err = native_window_api_disconnect(lc->pNativeWindow,
                NATIVE_WINDOW_API_CPU);
        if (err != NO_ERROR) {
            loge("error pushing blank frames: api_disconnect failed: %s (%d)",
                    strerror(-err), -err);
            return err;
        }

        err = native_window_api_connect(lc->pNativeWindow,
                NATIVE_WINDOW_API_MEDIA);
        if (err != NO_ERROR) {
            loge("error pushing blank frames: api_connect failed: %s (%d)",
                    strerror(-err), -err);
            return err;
        }

        return 0;
    }
}

static int SendThreeBlackFrameToGpu(LayerCtrlContext* lc)
{
    logd("SendThreeBlackFrameToGpu()");
    
    ANativeWindowBuffer* pWindowBuf;
    void*                pDataBuf;
    int                  i;
    int                  err;

    //* it just work on A80-box and H8         
#if((CONFIG_CHIP==OPTION_CHIP_1639 || CONFIG_CHIP==OPTION_CHIP_1673 || \
	CONFIG_CHIP == OPTION_CHIP_1680 || CONFIG_CHIP == OPTION_CHIP_1689) && \
	(CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))
    for(i = 0;i < NUM_OF_PICTURES_KEEP_IN_LIST;i++)
    {
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
            return -1;
        }
        lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

        //* lock the data buffer.
        {
            GraphicBufferMapper& mapper = GraphicBufferMapper::get();
            Rect bounds(lc->nWidth, lc->nHeight);
            mapper.lock(pWindowBuf->handle, /*lc->nUsage*/GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &pDataBuf);
			logd("mapper %p", pDataBuf);
        }

        if (pDataBuf) {
            memset((char*)pDataBuf,0x10,(pWindowBuf->height * pWindowBuf->stride));
            memset((char*)pDataBuf + pWindowBuf->height * pWindowBuf->stride,0x80,(pWindowBuf->height * pWindowBuf->stride)/2);
        }
        
        int nBufAddr[7] = {0};
        //nBufAddr[6] = HAL_PIXEL_FORMAT_AW_NV12;
        nBufAddr[6] = HAL_PIXEL_FORMAT_AW_FORCE_GPU;
        lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO,	nBufAddr[0], nBufAddr[1],
        nBufAddr[2], nBufAddr[3], nBufAddr[4], nBufAddr[5], nBufAddr[6]);
        
        //* unlock the buffer.
        {
            GraphicBufferMapper& mapper = GraphicBufferMapper::get();
            mapper.unlock(pWindowBuf->handle);
        }

        lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

    }
#endif
    return 0;
}

// dequeue all nativewindow buffer from BufferQueue, then cancle all.
// this can make sure all buffer queued by player to render by surfaceflinger.
static int MakeSureFramesToShow(LayerCtrlContext* lc)
{
	logd("MakeSureFramesToShow()");
    
    ANativeWindowBuffer* pWindowBuf[32];
    void*                pDataBuf;
    int                  i;
    int                  err;
	int					 bufCnt = lc->nGpuBufferCount;
     
    for(i = 0;i < bufCnt -1; i++)
    {
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf[i]);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
            break;
        }
        
    	logv("dequeue i = %d, handle: 0x%x", i, pWindowBuf[i]->handle);

    }

    for(i--; i >= 0; i--)
    {        
    	logv("cancel i = %d, handle: 0x%x", i, pWindowBuf[i]->handle);
        lc->pNativeWindow->cancelBuffer(lc->pNativeWindow, pWindowBuf[i], -1);
    }
	
    return 0;
}

//* set usage, scaling_mode, pixelFormat, buffers_geometry, buffers_count, crop
static int SetLayerParam(LayerCtrlContext* lc)
{
    logd("SetLayerParam: PixelFormat(%d), nW(%d), nH(%d), leftoff(%d), topoff(%d)",
          lc->eReceivePixelFormat,lc->nWidth,
          lc->nHeight,lc->nLeftOff,lc->nTopOff);
    logd("SetLayerParam: dispW(%d), dispH(%d), buffercount(%d), bProtectFlag(%d), bIsSoftDecoderFlag(%d)",
          lc->nDisplayWidth,lc->nDisplayHeight,lc->nGpuBufferCount,
          lc->bProtectFlag,lc->bIsSoftDecoderFlag);
    
    int          pixelFormat;
    unsigned int nGpuBufWidth;
    unsigned int nGpuBufHeight;
    Rect         crop;
    lc->nUsage   = 0;
	
    if(lc->bProtectFlag == 1)
    {
        // Verify that the ANativeWindow sends images directly to
        // SurfaceFlinger.
        int err = -1;
        int queuesToNativeWindow = 0;
        err = lc->pNativeWindow->query(
                lc->pNativeWindow, NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER,
                &queuesToNativeWindow);
        if (err != 0) {
            loge("error authenticating native window: %d", err);
            return err;
        }
        if (queuesToNativeWindow != 1) {
            loge("native window could not be authenticated");
            return PERMISSION_DENIED;
        }
        logd("set usage to GRALLOC_USAGE_PROTECTED");
        lc->nUsage |= GRALLOC_USAGE_PROTECTED;
    }

	#if(USE_NEW_DISPLAY == 1)
    if(lc->bIsSoftDecoderFlag == 1)
    {
        //* gpu use this usage to malloc buffer with cache.
        lc->nUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
    }
    else
    {
        //* gpu use this usage to malloc continuous physical buffer.
        lc->nUsage |= GRALLOC_USAGE_HW_2D;
    }
	#endif
    
    switch(lc->eReceivePixelFormat)
    {
        case PIXEL_FORMAT_YV12:             //* why YV12 use this pixel format.
            pixelFormat = HAL_PIXEL_FORMAT_YV12;
            break;
        case PIXEL_FORMAT_NV21:
            pixelFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP;
            break;
        case PIXEL_FORMAT_NV12: //* display system do not support NV12.
        default:
        {
            loge("unsupported pixel format.");
            return -1;
            break;
        }
    }
    
    lc->nUsage |= GRALLOC_USAGE_SW_READ_NEVER     |
              // GRALLOC_USAGE_SW_WRITE_OFTEN    | 
              GRALLOC_USAGE_HW_TEXTURE        | 
              GRALLOC_USAGE_EXTERNAL_DISP;
    
    native_window_set_usage(lc->pNativeWindow,lc->nUsage);
    native_window_set_scaling_mode(lc->pNativeWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

    #if 0
    nGpuBufWidth  = (lc->nWidth + 15) & ~15;
    nGpuBufHeight = (lc->nHeight + 15) & ~15;
    
    //A10's GPU has a bug, we can avoid it
    if(nGpuBufHeight%8 != 0)
    {
        logw("original picture align_width[%d], height[%d] mod8 = %d", nGpuBufWidth, nGpuBufHeight, nGpuBufHeight%8);
        if((nGpuBufWidth*nGpuBufHeight)%256 != 0)
        {
            logw("original picture align_width[%d]*height[%d] mod 1024 = %d", 
                   nGpuBufWidth, nGpuBufHeight, (nGpuBufWidth*nGpuBufHeight)%1024);
            nGpuBufHeight = (nGpuBufHeight+7)&~7;
            logw("change picture height to [%d] when render to gpu", nGpuBufHeight);
        }
    }
    #endif
    
    nGpuBufWidth  = lc->nWidth;  //* restore nGpuBufWidth to mWidth;
    nGpuBufHeight = lc->nHeight;

    //* We should double the height if the video with two stream,
    //* so the nativeWindow will malloc double buffer
    if(lc->bVideoWithTwoStreamFlag == 1)
    {
        nGpuBufHeight = 2*nGpuBufHeight;
    }
    native_window_set_buffers_geometry(lc->pNativeWindow, nGpuBufWidth, nGpuBufHeight, pixelFormat);

    crop.left   = lc->nLeftOff;
    crop.top    = lc->nTopOff;
    crop.right  = lc->nLeftOff + lc->nDisplayWidth;
    crop.bottom = lc->nTopOff + lc->nDisplayHeight;
    lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, &crop);

    //* set the buffer count
    if(lc->nGpuBufferCount != 0)
        native_window_set_buffer_count(lc->pNativeWindow,lc->nGpuBufferCount);
    else
    {
        loge("error: the nativeWindow buffer Count is 0!");
        return -1;
    }
    
    return 0;
}

LayerCtrl* NewLayerInit(void* pNativeWindow, int bProtectedFlag)
{
    logv("LayerInit, pNativeWindow = %p",pNativeWindow);
    
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)malloc(sizeof(LayerCtrlContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerCtrlContext));

    lc->ionFd = -1;
    lc->ionFd = ion_open();

    logd("ion open fd = %d",lc->ionFd);
    if(lc->ionFd < -1)
    {
        loge("ion open fail ! ");
        return NULL;
    }

    lc->bProtectFlag  = bProtectedFlag;
    lc->pNativeWindow = (ANativeWindow*)pNativeWindow;
    return (LayerCtrl*)lc;
}


void NewLayerRelease(LayerCtrl* l, int bKeepPictureOnScreen)
{
    LayerCtrlContext* lc;
    VPictureNode*     nodePtr;
    int i;
    int ret;
    VideoPicture mPicBufInfo;

	CEDARX_UNUSE(bKeepPictureOnScreen);
	
    lc = (LayerCtrlContext*)l;

    memset(&mPicBufInfo, 0, sizeof(VideoPicture));

    logv("LayerRelease, ionFd = %d",lc->ionFd);

    #if(CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX)
    if(lc->bProtectFlag == 0)
	{
		if(lc->bHoldLastPictureFlag == false/* && lc->bFirstFrameIsShowed == 1*/)
		{
			SendThreeBlackFrameToGpu(lc);
            MakeSureFramesToShow(lc);
		}
    }
    #endif

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        //* we should queue buffer which had dequeued to gpu
        if(lc->mGpuBufferInfo[i].nDequeueFlag == 1)
        {
            //* unlock the buffer.
			ANativeWindowBuffer* pWindowBuf = lc->mGpuBufferInfo[i].pWindowBuf;
	        GraphicBufferMapper& mapper = GraphicBufferMapper::get();
	        mapper.unlock(pWindowBuf->handle);
			
	        lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
			lc->mGpuBufferInfo[i].nDequeueFlag = 0;
        }

#if (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0)

        if(lc->mGpuBufferInfo[i].handle_ion != 0)
        {
            logd("ion_free: handle_ion[%d] = %p",i,lc->mGpuBufferInfo[i].handle_ion);
            ion_free(lc->ionFd,lc->mGpuBufferInfo[i].handle_ion);
        }
#else
		if(lc->mGpuBufferInfo[i].handle_ion != NULL)
		{
			logd("ion_free: handle_ion[%d] = %p",i,lc->mGpuBufferInfo[i].handle_ion);
			ion_free(lc->ionFd,lc->mGpuBufferInfo[i].handle_ion);
		}
#endif

    }

    if(lc->ionFd > 0)
    {
        ion_close(lc->ionFd);
    }

    if(lc->bProtectFlag == 1 /*|| lc->bHoldLastPictureFlag == 0*/)
    {
        ret = pushBlankBuffersToNativeWindow(lc);
        if(ret != 0)
        {
            loge("pushBlankBuffersToNativeWindow appear error!: ret = %d",ret);
        }
    }
    
    free(lc);    
}

void NewLayerResetNativeWindow(LayerCtrl* l,void* pNativeWindow)
{
    logd("LayerResetNativeWindow : %p ",pNativeWindow);
    
    LayerCtrlContext* lc;
    VideoPicture mPicBufInfo;

    lc = (LayerCtrlContext*)l; 

    memset(&mPicBufInfo, 0, sizeof(VideoPicture));

    //* we should queue buffer which had dequeued to gpu
    int i;
    for(i = 0; i < GPU_BUFFER_NUM; i++)
    {
        if(lc->mGpuBufferInfo[i].nDequeueFlag == 1)
        {
        	//* unlock the buffer.
			ANativeWindowBuffer* pWindowBuf = lc->mGpuBufferInfo[i].pWindowBuf;
	        GraphicBufferMapper& mapper = GraphicBufferMapper::get();
	        mapper.unlock(pWindowBuf->handle);
			
	        lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
			lc->mGpuBufferInfo[i].nDequeueFlag = 0;
        }
    }

    memset(&lc->mGpuBufferInfo,0,sizeof(GpuBufferInfoT)*GPU_BUFFER_NUM);

    lc->pNativeWindow = (ANativeWindow*)pNativeWindow;

    if(lc->pNativeWindow != NULL)
        lc->bLayerInitialized = 0;
    
    return ;
}

int NewLayerSetExpectPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;

    logv("Layer set expected pixel format, format = %d", (int)ePixelFormat);
    
    if(ePixelFormat == PIXEL_FORMAT_NV12 || 
       ePixelFormat == PIXEL_FORMAT_NV21 ||
       ePixelFormat == PIXEL_FORMAT_YV12)           //* add new pixel formats supported by gpu here.
    {
        lc->eReceivePixelFormat = ePixelFormat;
    }
    else
    {
        logv("receive pixel format is %d, not match.", lc->eReceivePixelFormat);
        return -1;
    }

    //* on A83-pad and A83-box , the address should 4k align when format is NV21
#if(CONFIG_CHIP == OPTION_CHIP_1673)
    if(lc->eReceivePixelFormat == PIXEL_FORMAT_NV21)
        lc->b4KAlignFlag = 1;
#endif
    
    return 0;
}


int NewLayerSetPictureSize(LayerCtrl* l, int nWidth, int nHeight)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;

    logv("Layer set picture size, width = %d, height = %d", nWidth, nHeight);
    
    lc->nWidth         = nWidth;
    lc->nHeight        = nHeight;
    lc->nDisplayWidth  = nWidth;
    lc->nDisplayHeight = nHeight;
    lc->nLeftOff       = 0;
    lc->nTopOff        = 0;
    lc->bLayerInitialized = 0;
    
    return 0;
}


int NewLayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff, int nDisplayWidth, int nDisplayHeight)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;

    logv("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, displayHeight = %d",
        nLeftOff, nTopOff, nDisplayWidth, nDisplayHeight);
    int scaler = (lc->bVideoWithTwoStreamFlag == 1) ? 2 : 1;
    
    if(nDisplayWidth != 0 && nDisplayHeight != 0)
    {
        lc->nDisplayWidth     = nDisplayWidth;
        lc->nDisplayHeight    = nDisplayHeight;
        lc->nLeftOff          = nLeftOff;
        lc->nTopOff           = nTopOff;

        int displayHeight = (lc->bVideoWithTwoStreamFlag == 1) ? lc->nDisplayHeight : nDisplayHeight;

        if(lc->bLayerInitialized == 1)
        {
            Rect         crop;
            crop.left   = lc->nLeftOff;
            crop.top    = lc->nTopOff;
            crop.right  = lc->nLeftOff + lc->nDisplayWidth;
            crop.bottom = lc->nTopOff + displayHeight * scaler;
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, &crop);
        }
        
        return 0;
    }
    else
        return -1;
}

int NewLayerSetBufferCount(LayerCtrl* l, int nBufferCount)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetBufferCount: count = %d",nBufferCount);
    
//    lc->nGpuBufferCount = nBufferCount + NUM_OF_PICTURES_KEEP_IN_LIST;
    lc->nGpuBufferCount = nBufferCount;

    if(lc->nGpuBufferCount > GPU_BUFFER_NUM)
        lc->nGpuBufferCount = GPU_BUFFER_NUM;

	return lc->nGpuBufferCount;
}

int NewLayerSetVideoWithTwoStreamFlag(LayerCtrl* l, int bVideoWithTwoStreamFlag)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetIsTwoVideoStreamFlag, flag = %d",bVideoWithTwoStreamFlag);

    lc->bVideoWithTwoStreamFlag = bVideoWithTwoStreamFlag;

    return 0;
}

VideoPicture* NewLayerGetPicNode(LayerCtrl* l)
{
	LayerCtrlContext* lc;
	VideoPicture* pPicture = NULL;
	lc = (LayerCtrlContext*)l;
	for(int i = 0; i<NUM_OF_PICTURES_KEEP_IN_NODE; i++)
	{
		if(lc->picNodes[i].bUsed == 1)
		{
			lc->picNodes[i].bUsed = 0;
			pPicture = lc->picNodes[i].pPicture;
			break;
		}
	}

	return pPicture;
}

int NewLayerGetAddedPicturesCount(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
	return NUM_OF_PICTURES_KEEP_IN_LIST;
}

#if(CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX)
int NewLayerGetDisplayFPS(LayerCtrl* l)
{
	enum{
		DISPLAY_CMD_GETDISPFPS = 0x29,
	};

	int dispFPS = 0;

	LayerCtrlContext* lc;

	lc = (LayerCtrlContext*)l;

	if(lc->pNativeWindow != NULL)
		dispFPS = lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_GETPARAMETER, DISPLAY_CMD_GETDISPFPS);

	if (dispFPS <= 0) /* DISPLAY_CMD_GETDISPFPS not support, assume a nice fps */
		dispFPS = 60;

	return dispFPS;
}
#else
int NewLayerGetDisplayFPS(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function NewLayerGetDisplayFPS");
    return -1;
}
#endif 

int NewLayerSetIsSoftDecoderFlag(LayerCtrl* l, int bIsSoftDecoderFlag)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetIsSoftDecoderFlag, flag = %d",bIsSoftDecoderFlag);

    lc->bIsSoftDecoderFlag = bIsSoftDecoderFlag;

    return 0;
}

int NewLayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    native_window_set_buffers_timestamp(lc->pNativeWindow, nPtsAbs);

    return 0;
}

//*re-design this function
int NewLayerDequeueBuffer(LayerCtrl* l,VideoPicture** ppVideoPicture, int bInitFlag)
{
	logv("LayerDequeueBuffer, *ppVideoPicture(%p),bInitFlag(%d)",
		*ppVideoPicture,bInitFlag);
	
    LayerCtrlContext* lc;
	VideoPicture* pPicture = NULL;
    
    lc = (LayerCtrlContext*)l;

    logv("LayerDequeueBuffer");    

    //* dequeue a buffer from the native window object, set it to a picture buffer wrapper.
    ANativeWindowBuffer* pWindowBuf = NULL;
	
#if (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0)
	ion_user_handle_t handle_ion = 0;
#else
	struct ion_handle *handle_ion  = NULL;
#endif

    void*   pDataBuf    = NULL;
    int     err = -1;
    int     i   = 0;
    int     nNeedCancelFlag = 0;
    int     nCancelNum = 0;
    int     nCancelIndex[GPU_BUFFER_NUM] = {-1};
    uintptr_t  nPhyaddress = -1;

    if(lc->pNativeWindow == NULL)
    {
        logw("pNativeWindow is null when dequeue buffer");
        return -1;
    }
        
    if(lc->bLayerInitialized == 0)
    {
        if(SetLayerParam(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }        
        lc->bLayerInitialized = 1;        
    }
    
dequeue_buffer:
    
    //* dequeue a buffer from the nativeWindow object.
    err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
    if(err != 0)
    {
        logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
        return -1;
    }

	if(bInitFlag == 1)
	{
		for(i = 0; i < lc->nGpuBufferCount; i++)
	    {
	        if(lc->mGpuBufferInfo[i].nUsedFlag == 1
	           && lc->mGpuBufferInfo[i].pWindowBuf == pWindowBuf)
	        {
	            nNeedCancelFlag = 1;
	            nCancelIndex[nCancelNum] = i;
	            nCancelNum++;
	            logv("the buffer have not return ,dequeue agagin! : %p",pWindowBuf);
	            goto dequeue_buffer;
	        }
	    }

	    if(nNeedCancelFlag == 1)
	    {
	        for(i = 0;i<nCancelNum;i++)
	        {
	            int nIndex = nCancelIndex[i];
	            ANativeWindowBuffer* pTmpWindowBuf = lc->mGpuBufferInfo[nIndex].pWindowBuf;
	            lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pTmpWindowBuf);
	        }
	        nCancelNum = 0;
	        nNeedCancelFlag = 0;
	    }
	}

    lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

    //* lock the data buffer.
    {
        GraphicBufferMapper& mapper = GraphicBufferMapper::get();
        Rect bounds(lc->nWidth, lc->nHeight);
        #if(CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX && CONFIG_CHIP != OPTION_CHIP_1689) //* ÁÙÊ±ÓÃ£¬pd¸ú½ø¡£
        mapper.lock(pWindowBuf->handle,lc->nUsage, bounds, &pDataBuf);
        #else
        mapper.lock(pWindowBuf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &pDataBuf);
        #endif
    }

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        if(lc->mGpuBufferInfo[i].pWindowBuf == NULL)
        {
//for mali GPU
#if(GPU_TYPE_MALI == 1)
			private_handle_t* hnd = (private_handle_t *)(pWindowBuf->handle);
#else 
            IMG_native_handle_t* hnd = (IMG_native_handle_t*)(pWindowBuf->handle);
#endif

            if(hnd != NULL)
            {
#if(GPU_TYPE_MALI == 1)
				ion_import(lc->ionFd, hnd->share_fd, &handle_ion);
#else
				ion_import(lc->ionFd, hnd->fd[0], &handle_ion);
#endif
            }
            else
            {
                logd("the hnd is wrong : hnd = %p",hnd);
                return -1;
            }

            //* we should not get the phyaddr if it is software decoder
            if(lc->bIsSoftDecoderFlag == 0)
            {
                if(lc->ionFd > 0)
                    nPhyaddress = ion_getphyadr(lc->ionFd, handle_ion);
                else
                {
                    logd("the ion fd is wrong : fd = %d",lc->ionFd);
                    return -1;
                }

                nPhyaddress -= PHY_OFFSET;
            }

            if(lc->bVideoWithTwoStreamFlag == 1)
            {
                initPartialGpuBuffer((char*)pDataBuf,pWindowBuf,lc);
            }

            lc->mGpuBufferInfo[i].pWindowBuf   = pWindowBuf;
            lc->mGpuBufferInfo[i].handle_ion   = handle_ion;
            lc->mGpuBufferInfo[i].pBufVirAddr  = (char*)pDataBuf;
            lc->mGpuBufferInfo[i].pBufPhyAddr  = (char*)nPhyaddress;
            lc->mGpuBufferInfo[i].nUsedFlag    = 1;
            lc->mGpuBufferInfo[i].nDequeueFlag = 1;
            break;
        }
        else if(lc->mGpuBufferInfo[i].pWindowBuf == pWindowBuf)
        {
            lc->mGpuBufferInfo[i].nUsedFlag    = 1;
            lc->mGpuBufferInfo[i].nDequeueFlag = 1;
            break;
        }
    }
    
    if(i == lc->nGpuBufferCount)
    {
        loge("not enouth gpu buffer , should not run here");
        abort();
    }

	//* dequeue buffer for the first time, we should not dequeue from picNode
	if(bInitFlag == 1)
	{
		pPicture = *ppVideoPicture;
		//* set the buffer address
	    pPicture->pData0       = lc->mGpuBufferInfo[i].pBufVirAddr;
	    pPicture->pData1       = pPicture->pData0 + (pWindowBuf->height * pWindowBuf->stride);
	    pPicture->pData2       = pPicture->pData1 + (pWindowBuf->height * pWindowBuf->stride)/4;
	    pPicture->phyYBufAddr  = (uintptr_t)lc->mGpuBufferInfo[i].pBufPhyAddr;
	    pPicture->phyCBufAddr  = pPicture->phyYBufAddr + (pWindowBuf->height * pWindowBuf->stride);
	    pPicture->nBufId	   = i;
	    pPicture->pPrivate     = (void*)(uintptr_t)lc->mGpuBufferInfo[i].handle_ion;
	    pPicture->ePixelFormat = lc->eReceivePixelFormat;
	    pPicture->nWidth       = pWindowBuf->stride;
	    pPicture->nHeight      = pWindowBuf->height;
	    pPicture->nLineStride  = pWindowBuf->stride;

	    if(lc->b4KAlignFlag == 1)
		{
		    uintptr_t tmpAddr = (uintptr_t)pPicture->pData1;
		    tmpAddr     = (tmpAddr + 4095) & ~4095;		    
		    pPicture->pData1      = (char *)tmpAddr;
		    pPicture->phyCBufAddr = (pPicture->phyCBufAddr + 4095) & ~4095;
		}
	}
	else
	{
		for(i = 0; i<NUM_OF_PICTURES_KEEP_IN_NODE; i++)
		{
			logv("** dequeue , i(%d), used(%d), pData0(%p),pDataBuf(%p), pPicture(%p)",
					i,lc->picNodes[i].bUsed, lc->picNodes[i].pPicture->pData0,(char*)pDataBuf,lc->picNodes[i].pPicture);
			if(lc->picNodes[i].bUsed == 1 
			   && lc->picNodes[i].pPicture != NULL
			   && lc->picNodes[i].pPicture->pData0 == (char*)pDataBuf)
			{				
				pPicture = lc->picNodes[i].pPicture ;
				lc->picNodes[i].bUsed = 0;
				break;
			}				
		}
		if(i == NUM_OF_PICTURES_KEEP_IN_NODE)
		{
			loge("hava no unused picture in the picNode, pDataBuf = %p",pDataBuf);
			return -1;
		}
	}        
	
	*ppVideoPicture = pPicture;
    return 0;
}
int NewLayerQueueBuffer(LayerCtrl* l,VideoPicture* pInPicture, int bValid)
{
	logv("LayerQueueBuffer: pInPicture = %p, pData0 = %p",
			pInPicture,pInPicture->pData0);
    ANativeWindowBuffer* pWindowBuf = NULL;
    LayerCtrlContext* lc  = NULL;
    int               i   = 0;
    char*             pBuf   = NULL;
    int               nBufId = -1;
    
    lc = (LayerCtrlContext*)l;

    if(lc->bLayerInitialized == 0)
    {
        if(SetLayerParam(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }
        
        lc->bLayerInitialized = 1;
        
    }

	for(i = 0; i<NUM_OF_PICTURES_KEEP_IN_NODE; i++)
	{
		if(lc->picNodes[i].bUsed == 0)
		{
			lc->picNodes[i].bUsed = 1;
			lc->picNodes[i].pPicture = pInPicture;
			break;
		}
	}

	if(i == NUM_OF_PICTURES_KEEP_IN_NODE)
	{
		loge("*** picNode is full when queue buffer");
		return -1;
	}
	//loge("*** LayerQueueBuffer pInPicture = %p, bValid = %d",pInPicture,bValid);
    pBuf   = (char*)pInPicture->phyYBufAddr;
    nBufId = pInPicture->nBufId;
    


    pWindowBuf = lc->mGpuBufferInfo[nBufId].pWindowBuf;
    
    //* unlock the buffer.
    {
        GraphicBufferMapper& mapper = GraphicBufferMapper::get();
        mapper.unlock(pWindowBuf->handle);
    }

    if(bValid == 1)
        lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    else
        lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

    lc->mGpuBufferInfo[nBufId].nDequeueFlag = 0;
    
    logv("******LayerQueueBuffer finish!");
    return 0;
}

int NewLayerReleaseBuffer(LayerCtrl* l,VideoPicture* pPicture)
{
	logv("***LayerReleaseBuffer");
	LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
#if (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0)
	ion_user_handle_t handle_ion = 0;
	handle_ion = (ion_user_handle_t)(uintptr_t)pPicture->pPrivate;
	ion_free(lc->ionFd,handle_ion);
#else
	struct ion_handle *handle_ion  = NULL;
	handle_ion = (struct ion_handle *)pPicture->pPrivate;
	ion_free(lc->ionFd,handle_ion);
#endif	

	return 0;
}


int NewLayerCtrlShowVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;
    
    logv("LayerCtrlShowVideo, current show flag = %d", lc->bLayerShowed);
    
    if(lc->bLayerShowed == 0)
    {
        if(lc->pNativeWindow != NULL)
        {
        	lc->bLayerShowed = 1;
            lc->pNativeWindow->perform(lc->pNativeWindow,
                                       NATIVE_WINDOW_SETPARAMETER,
                                       HWC_LAYER_SHOW,
                                       1);
        }
        else
        {
            logw("the nativeWindow is null when call LayerCtrlShowVideo()");
            return -1;
        }
    }
    return 0;
}

int NewLayerCtrlHideVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;
    
    logv("LayerCtrlHideVideo, current show flag = %d", lc->bLayerShowed);
    
    if(lc->bLayerShowed == 1)
    {
        if(lc->pNativeWindow != NULL)
        {
    	lc->bLayerShowed = 0;
        lc->pNativeWindow->perform(lc->pNativeWindow,
                                       NATIVE_WINDOW_SETPARAMETER,
                                       HWC_LAYER_SHOW,
                                       0);
        }
        else
        {
            logw("the nativeWindow is null when call LayerCtrlHideVideo()");
            return -1;
        }
    }
    return 0;
}

int NewLayerCtrlIsVideoShow(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerCtrlIsVideoShow : bLayerShowed = %d",lc->bLayerShowed);
    
    return lc->bLayerShowed;
}

int NewLayerCtrlHoldLastPicture(LayerCtrl* l, int bHold)
{
    logv("LayerCtrlHoldLastPicture, bHold = %d", bHold);
    //*TODO
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    lc->bHoldLastPictureFlag = bHold;
    
    return 0;
}

int NewLayerSetRenderToHardwareFlag(LayerCtrl* l,int bFlag)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(bFlag);
    logw("not implement the function NewLayerSetRenderToHardwareFlag");
    return -1;
}

enum EPIXELFORMAT NewLayerGetPixelFormat(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function NewLayerGetPixelFormat");
    return PIXEL_FORMAT_DEFAULT;
}

int NewLayerSetDeinterlaceFlag(LayerCtrl* l,int bFlag)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(bFlag);
    logw("not implement the function NewLayerSetDeinterlaceFlag");
    return -1;
}

int NewLayerDequeue3DBuffer(LayerCtrl* l, VideoPicture** ppBuf0, VideoPicture** ppBuf1)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(ppBuf0);
    CDX_PLAYER_UNUSE(ppBuf1);
    logw("not implement the function NewLayerDequeue3DBuffer");
    return -1;
}

int NewLayerQueue3DBuffer(LayerCtrl* l, VideoPicture* pBuf0, VideoPicture* pBuf1, int bValid)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(pBuf0);
    CDX_PLAYER_UNUSE(pBuf1);
    CDX_PLAYER_UNUSE(bValid);
    logw("not implement the function NewLayerQueue3DBuffer");
    return -1;
}

int NewLayerGetRotationAngle(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function NewLayerGetRotationAngle");
    return -1;
}

int NewLayerSetCallback(LayerCtrl* l, LayerCtlCallback callback, void* pUserData)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(callback);
    CDX_PLAYER_UNUSE(pUserData);
    logw("not implement the function NewLayerSetCallback");
    return -1;
}

int NewLayerSetPicture3DMode(LayerCtrl* l, enum EPICTURE3DMODE ePicture3DMode)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(ePicture3DMode);
    logw("not implement the function NewLayerSetPicture3DMode");
    return -1;
}

enum EPICTURE3DMODE NewLayerGetPicture3DMode(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function NewLayerGetPicture3DMode");
    return PICTURE_3D_MODE_NONE;
}

int NewLayerSetDisplay3DMode(LayerCtrl* l, enum EDISPLAY3DMODE eDisplay3DMode)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(eDisplay3DMode);
    logw("not implement the function NewLayerSetDisplay3DMode");
    return -1;
}

enum EDISPLAY3DMODE NewLayerGetDisplay3DMode(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function NewLayerGetDisplay3DMode");
    return DISPLAY_3D_MODE_2D;
}

LayerControlOpsT mNewLayerControlOps = 
{
    LayerInit:                       NewLayerInit                      ,//
    LayerRelease:                    NewLayerRelease                   ,
    LayerSetExpectPixelFormat:       NewLayerSetExpectPixelFormat      ,
    LayerGetPixelFormat:             NewLayerGetPixelFormat            ,
    LayerSetPictureSize:             NewLayerSetPictureSize            ,
    LayerSetDisplayRegion:           NewLayerSetDisplayRegion          ,
    LayerSetBufferTimeStamp:         NewLayerSetBufferTimeStamp        ,
    LayerDequeueBuffer:              NewLayerDequeueBuffer             ,
    LayerQueueBuffer:                NewLayerQueueBuffer               ,
    LayerCtrlHideVideo:              NewLayerCtrlHideVideo             ,
    LayerCtrlShowVideo:              NewLayerCtrlShowVideo             ,
    LayerCtrlIsVideoShow:		     NewLayerCtrlIsVideoShow           ,
    LayerCtrlHoldLastPicture:        NewLayerCtrlHoldLastPicture       ,
    LayerSetRenderToHardwareFlag:    NewLayerSetRenderToHardwareFlag   ,
    LayerSetDeinterlaceFlag:         NewLayerSetDeinterlaceFlag        ,
    LayerSetPicture3DMode:           NewLayerSetPicture3DMode          ,
    LayerGetPicture3DMode:           NewLayerGetPicture3DMode          ,
    LayerSetDisplay3DMode:           NewLayerSetDisplay3DMode          ,
    LayerGetDisplay3DMode:           NewLayerGetDisplay3DMode          ,
    LayerDequeue3DBuffer:            NewLayerDequeue3DBuffer           ,
    LayerQueue3DBuffer:              NewLayerQueue3DBuffer             ,
    LayerGetRotationAngle:           NewLayerGetRotationAngle          ,
    LayerSetCallback:                NewLayerSetCallback               ,
    LayerSetBufferCount:             NewLayerSetBufferCount            ,
    LayerSetVideoWithTwoStreamFlag:  NewLayerSetVideoWithTwoStreamFlag ,        
    LayerSetIsSoftDecoderFlag:       NewLayerSetIsSoftDecoderFlag      ,
    LayerResetNativeWindow:          NewLayerResetNativeWindow         ,
    LayerReleaseBuffer:              NewLayerReleaseBuffer             ,
    LayerGetPicNode:                 NewLayerGetPicNode                ,
    LayerGetAddedPicturesCount:      NewLayerGetAddedPicturesCount     ,
    LayerGetDisplayFPS:              NewLayerGetDisplayFPS             
   
};


