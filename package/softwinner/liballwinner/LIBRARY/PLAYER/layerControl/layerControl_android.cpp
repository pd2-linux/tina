
#include "log.h"
#include "config.h"

#if CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_2
#include <gui/ISurfaceTexture.h>
#elif (CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_4 || CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0)
#include <gui/Surface.h>
#else
    #error "invalid configuration of os version."
#endif

#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include "layerControl.h"
#include "deinterlace.h"
#include "memoryAdapter.h"
#include <hardware/hwcomposer.h>

#if(CONFIG_CHIP != OPTION_CHIP_1651)
#include <hardware/hal_public.h>
#endif
#include <linux/ion.h>
#include <ion/ion.h>

#define DEBUG_DUMP_PIC 0

#if CONFIG_CHIP == OPTION_CHIP_1639
#define PHY_OFFSET 0x20000000
#else
#define PHY_OFFSET 0x40000000
#endif
static VideoPicture* gLastPicture = NULL;


typedef struct VPictureNode_t VPictureNode;
struct VPictureNode_t
{
    VideoPicture* pPicture;
    VideoPicture* pSecondPictureOf3D;
    buffer_handle_t handle;
};

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
    int                  bRenderToHardwareLayer;
    enum EPICTURE3DMODE  ePicture3DMode;
    enum EDISPLAY3DMODE  eDisplay3DMode;
    int                  bLayerInitialized;
    int                  bLayerShowed;
    int                  bDeinterlaceFlag;		// 0: do not de-i, 1: hw de-i, 2: sw de-i
    int                  nPreNativeWindowOutputType;
	int 				 bFirstFrameIsShowed; // 0->first not showed, 1->is already showed
	int					 nOutputTypeChanged;	//0:no change  1:gpu->0 copy
    
    //* use when render to gpu.
    VideoPicture         bufferWrappers[32];
    int                  bBufferWrapperUsed[32];
    ANativeWindowBuffer* pWindowBufs[32];
    
    //* use when render derect to hardware layer.
    VPictureNode         *pKeepPicNode;

	bool				 keepLastFrameFlag;
    int                  nGpuYAlign;
    int                  nGpuCAlign;
    int picDump;
}LayerCtrlContext;

static int SetLayerParam(LayerCtrlContext* lc);
static void setHwcLayerPictureInfo(libhwclayerpara_t* pHwcLayerPictureInfo, 
                                   VideoPicture*      pPicture,
                                   VideoPicture*      pSecondPictureOf3D);
static int SendThreeBlackFrameToGpu(LayerCtrlContext* lc);

LayerCtrl* LayerInit(void* pNativeWindow, int bProtectedFlag)
{
    CDX_PLAYER_UNUSE(bProtectedFlag);
    LayerCtrlContext* lc;
    
    logv("LayerInit.");
    
    lc = (LayerCtrlContext*)malloc(sizeof(LayerCtrlContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerCtrlContext));
    lc->picDump = 0;
    
    lc->pNativeWindow = (ANativeWindow*)pNativeWindow;
    
    int nOutputType = lc->pNativeWindow->perform(lc->pNativeWindow, 
                                       NATIVE_WINDOW_GETPARAMETER, 
                                       NATIVE_WINDOW_CMD_GET_SURFACE_TEXTURE_TYPE, 
                                       0);
    lc->nPreNativeWindowOutputType = nOutputType;
    
    if(0 == nOutputType)                                       
    {
        lc->eReceivePixelFormat    = PIXEL_FORMAT_YV12;
        lc->bRenderToHardwareLayer = 0;
    }
    else
    {
#if ((CONFIG_CHIP==OPTION_CHIP_1639) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))      //* display system do not support MB32.
        lc->eReceivePixelFormat      = PIXEL_FORMAT_YUV_MB32_420;
        lc->bRenderToHardwareLayer   = 1;
#elif ((CONFIG_CHIP==OPTION_CHIP_1673) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))
		lc->eReceivePixelFormat      = PIXEL_FORMAT_YV12;
        lc->bRenderToHardwareLayer   = 1;
#elif ((CONFIG_CHIP == OPTION_CHIP_1680) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))
        lc->eReceivePixelFormat 	 = PIXEL_FORMAT_NV21;
        lc->bRenderToHardwareLayer	 = 1;
#else
    	lc->eReceivePixelFormat    = PIXEL_FORMAT_YV12;
        lc->bRenderToHardwareLayer = 0;
#endif
    }

    //* also 0 copy on A20    
#if(CONFIG_CHIP==OPTION_CHIP_1651) 
	lc->eReceivePixelFormat    = PIXEL_FORMAT_YV12;
    lc->bRenderToHardwareLayer = 1;
#endif

#if(CONFIG_CHIP==OPTION_CHIP_1680 || CONFIG_CHIP==OPTION_CHIP_1667)  //* on 1680, Y-Align is 16, C-Align is 16,
    lc->nGpuYAlign = 16;
    lc->nGpuCAlign = 16;
#elif(CONFIG_CHIP==OPTION_CHIP_1673)//* on 1673, Y-Align is 32, C-Align is 16,
    lc->nGpuYAlign = 32;
    lc->nGpuCAlign = 16;
#else                               //* on others, Y-Align is 16, C-Align is 8,
    lc->nGpuYAlign = 16;
    lc->nGpuCAlign = 8;
#endif    
    //* open the memory module, we need get physical address of a picture buffer 
    //* by MemAdapterGetPhysicAddress().
    if(gLastPicture == NULL)
        MemAdapterOpen();

	lc->keepLastFrameFlag = false;

	lc->pKeepPicNode = (VPictureNode*)malloc(NUM_OF_PICTURES_KEEP_IN_LIST*sizeof(VPictureNode));
	if(lc->pKeepPicNode == NULL)
	{
		loge("malloc failed!");
		free(lc);
		return NULL;
	}
	memset(lc->pKeepPicNode,0,NUM_OF_PICTURES_KEEP_IN_LIST*sizeof(VPictureNode));
	
    return (LayerCtrl*)lc;
}


void LayerRelease(LayerCtrl* l, int bKeepPictureOnScreen)
{
	CDX_PLAYER_UNUSE(bKeepPictureOnScreen);
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;

    logv("Layer release");
    
#if((CONFIG_CHIP==OPTION_CHIP_1639 || CONFIG_CHIP==OPTION_CHIP_1673 || CONFIG_CHIP == OPTION_CHIP_1680) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))
    //* if it is 0-copy, we should send 3 black frame to gpu when exit for
    //* avoiding a bug of GPU(Green screen when switch video).
    //* When gpu fix this bug, we should remove the codec

    //* We should ensure that gpu had displayed at least one picture when
    //* send-3-black-frame, or some param in gpu will not right,such as w, h
    if(lc->keepLastFrameFlag == false && lc->bFirstFrameIsShowed == 1)
    {
		SendThreeBlackFrameToGpu(lc);
    }

#endif

    //* return pictures.
	for(int i = 0; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
    {
    	if (lc->pKeepPicNode[i].pPicture != NULL)
    	{
        	lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pPicture);
    	}
    }

	if(lc->pKeepPicNode)
	{
		free(lc->pKeepPicNode);
		lc->pKeepPicNode = NULL;
	}
    
    //* free the memory module.
    if(gLastPicture == NULL)
        MemAdapterClose();
    
    free(lc);    
}

int LayerSetRenderToHardwareFlag(LayerCtrl* l,int bFlag)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;

    logv("LayerSetRenderToHardware : %d",bFlag);
    
    if(lc == NULL)
    {
        loge("error: the lc is null!");
        return -1;
    }
    lc->bRenderToHardwareLayer = bFlag;
    return 0;
}

int LayerSetDeinterlaceFlag(LayerCtrl* l,int bFlag)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;

    logv("LayerSetDeinterlaceFlag : %d",bFlag);
    
    if(lc == NULL)
    {
        loge("error: the lc is null!");
        return -1;
    }
    lc->bDeinterlaceFlag = bFlag;
    return 0;
}

int LayerSetExpectPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer set expected pixel format, format = %d", (int)ePixelFormat);
    
    if(0 == lc->bRenderToHardwareLayer)
    {
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
    }
    else
    {
        //* reder directly to hardware layer.
        if(ePixelFormat == PIXEL_FORMAT_YUV_MB32_420   ||
           ePixelFormat == PIXEL_FORMAT_YUV_MB32_422   ||
           ePixelFormat == PIXEL_FORMAT_YUV_MB32_444   ||
           ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420 ||
           ePixelFormat == PIXEL_FORMAT_YV12           ||
           ePixelFormat == PIXEL_FORMAT_NV12           ||
           ePixelFormat == PIXEL_FORMAT_NV21)   //* add new pixel formats supported by hardware layer here.
        {
            lc->eReceivePixelFormat = ePixelFormat;
        }
        else
        {
            logv("receive pixel format is %d, not match.", lc->eReceivePixelFormat);
            return -1;
        }
    }
    
    return 0;
}


enum EPIXELFORMAT LayerGetPixelFormat(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer get pixel format, return %d", lc->eReceivePixelFormat);
    
    return lc->eReceivePixelFormat;
}


int LayerSetPictureSize(LayerCtrl* l, int nWidth, int nHeight)
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


int LayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff, int nDisplayWidth, int nDisplayHeight)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, displayHeight = %d",
        nLeftOff, nTopOff, nDisplayWidth, nDisplayHeight);
    
    if(nDisplayWidth != 0 && nDisplayHeight != 0)
    {
        lc->nDisplayWidth  = nDisplayWidth;
        lc->nDisplayHeight = nDisplayHeight;
        lc->nLeftOff       = nLeftOff;
        lc->nTopOff        = nTopOff;
        return 0;
    }
    else
        return -1;
}

//* now, we not use set-3D-mode interface
#if 0
int LayerSetPicture3DMode(LayerCtrl* l, enum EPICTURE3DMODE ePicture3DMode)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer set picture 3d mode, mode = %d", (int)ePicture3DMode);
    
    lc->ePicture3DMode = ePicture3DMode;
    
    return 0;
}


enum EPICTURE3DMODE LayerGetPicture3DMode(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer get picture 3d mode, mode = %d", (int)lc->ePicture3DMode);
    
    return lc->ePicture3DMode;
}


int LayerSetDisplay3DMode(LayerCtrl* l, enum EDISPLAY3DMODE eDisplay3DMode)
{
    LayerCtrlContext* lc;
    video3Dinfo_t     hwc3DInfo;
    int               err;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer set display 3d mode, mode = %d", (int)eDisplay3DMode);
    
    lc->eDisplay3DMode = eDisplay3DMode;
    
    if(lc->bLayerInitialized == 0)
        return 0;
    
    hwc3DInfo.width  = lc->nWidth;
    hwc3DInfo.height = lc->nHeight;
    
    switch(lc->eReceivePixelFormat)
    {
        case PIXEL_FORMAT_YUV_PLANER_420:
        case PIXEL_FORMAT_YV12:             //* why YV12 use this pixel format.
            hwc3DInfo.format = HWC_FORMAT_YUV420PLANAR;
            break;
        case PIXEL_FORMAT_YUV_MB32_420:
            hwc3DInfo.format = HWC_FORMAT_MBYUV420;
            break;
        case PIXEL_FORMAT_YUV_PLANER_422:
            hwc3DInfo.format = HWC_FORMAT_MBYUV422;
            break;
        default:
        {
            loge("unsupported pixel format.");
            return -1;
            break;
        }
    }
    
    switch(lc->ePicture3DMode)
    {
        case PICTURE_3D_MODE_TWO_SEPERATED_PICTURE:
            hwc3DInfo.src_mode = HWC_3D_SRC_MODE_FP;
            break;
        case PICTURE_3D_MODE_SIDE_BY_SIDE:
            hwc3DInfo.src_mode = HWC_3D_SRC_MODE_SSH;
            break;
        case PICTURE_3D_MODE_TOP_TO_BOTTOM:
            hwc3DInfo.src_mode = HWC_3D_SRC_MODE_TB;
            break;
        case PICTURE_3D_MODE_LINE_INTERLEAVE:
            hwc3DInfo.src_mode = HWC_3D_SRC_MODE_LI;
            break;
        default:
            hwc3DInfo.src_mode = HWC_3D_SRC_MODE_NORMAL;
            break;
    }
    
    switch(eDisplay3DMode)
    {
        case DISPLAY_3D_MODE_3D:
            hwc3DInfo.display_mode = HWC_3D_OUT_MODE_HDMI_3D_1080P24_FP;
            break;
        case DISPLAY_3D_MODE_HALF_PICTURE:
            hwc3DInfo.display_mode = HWC_3D_OUT_MODE_2D;
            break;
        case DISPLAY_3D_MODE_2D:
        default:
            hwc3DInfo.display_mode = HWC_3D_OUT_MODE_ORIGINAL;
            break;
    }
    
    err = lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SETPARAMETER, HWC_LAYER_SET3DMODE, &hwc3DInfo);
    if(err == 0)
    {
        lc->eDisplay3DMode = eDisplay3DMode;
        return 0;
    }
    else
    {
        logw("set 3d mode fail to hardware layer.");
        return -1;
    }
}


enum EDISPLAY3DMODE LayerGetDisplay3DMode(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer get display 3d mode, mode = %d", (int)lc->eDisplay3DMode);
    
    return lc->eDisplay3DMode;
}
#endif

int LayerGetRotationAngle(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int nRotationAngle = 0;
    
    lc = (LayerCtrlContext*)l;

    if(lc == NULL)
    {
        loge("**the lc is null when get rotation angle!");
        return 0;
    }

    if(lc->pNativeWindow != NULL)
    {
        int nTransform     = 0;
        lc->pNativeWindow->query(lc->pNativeWindow, NATIVE_WINDOW_TRANSFORM_HINT,&nTransform);
        logv("################### nTransform = %d",nTransform);
        if(nTransform == 0)
        {
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_BUFFERS_TRANSFORM, 0);
            nRotationAngle = 0;
        }
        else if(nTransform == HAL_TRANSFORM_ROT_90)
        {
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_BUFFERS_TRANSFORM, 7);
            nRotationAngle = 90;
        }
        else if(nTransform == HAL_TRANSFORM_ROT_180)
        {
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_BUFFERS_TRANSFORM, 3);
            nRotationAngle = 180;
        }
        else if(nTransform == HAL_TRANSFORM_ROT_270)
        {
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_BUFFERS_TRANSFORM, 4);
            nRotationAngle = 270;
        }
        else
            nRotationAngle = 0;
    }
    else
        nRotationAngle = 0;
    
    logv("Layer get rotation angle , nRotationAngle = %d", nRotationAngle);

    return nRotationAngle;
    
}

int LayerSetCallback(LayerCtrl* l, LayerCtlCallback callback, void* pUserData)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    lc->callback  = callback;
    lc->pUserData = pUserData;
    return 0;
}


int LayerDequeueBuffer(LayerCtrl* l, VideoPicture** ppBuf,int bInitFlag)
{
    CDX_PLAYER_UNUSE(bInitFlag);
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    *ppBuf = NULL;

	enum EPIXELFORMAT eHwPixelFormat = PIXEL_FORMAT_YV12;
	enum EPIXELFORMAT eSwPixelFormat = PIXEL_FORMAT_YV12;

#if((CONFIG_CHIP==OPTION_CHIP_1639) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))
	eHwPixelFormat = PIXEL_FORMAT_YUV_MB32_420;
	eSwPixelFormat = PIXEL_FORMAT_YV12;
#elif ((CONFIG_CHIP==OPTION_CHIP_1673) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))
	eHwPixelFormat = PIXEL_FORMAT_YV12;
	eSwPixelFormat = PIXEL_FORMAT_YV12;
#elif ((CONFIG_CHIP==OPTION_CHIP_1680) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))
	eHwPixelFormat = PIXEL_FORMAT_NV21;
	eSwPixelFormat = PIXEL_FORMAT_NV21;
#endif

    //* check the NATIVE_WINDOW_GET_OUTPUT_TYPE, just on A80-box
    //* we should decide to 0-copy or not-0-copy rely on outputType every frame
#if(CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX && CONFIG_CHIP!=OPTION_CHIP_1651)
    //* deinterlace must not-0-copy, so we should not change it
    if(lc->bDeinterlaceFlag == 0)
    {
        int nCurOutputType = lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_GET_OUTPUT_TYPE);
        logv("nCurOutputType = %d",nCurOutputType);
        if(nCurOutputType != lc->nPreNativeWindowOutputType)
        {
            logd("output type have change : %d, %d",nCurOutputType,lc->nPreNativeWindowOutputType);
            if(nCurOutputType == 0)
            {
                lc->eReceivePixelFormat    = eSwPixelFormat;
                lc->bRenderToHardwareLayer = 0;
            }
            else
            {

				lc->eReceivePixelFormat      = eHwPixelFormat;
                lc->bRenderToHardwareLayer   = 1;
				lc->nOutputTypeChanged = 1;
            }
            lc->bLayerInitialized          = 0; //* setLayerParam again
            lc->nPreNativeWindowOutputType = nCurOutputType;
        }
    }
#endif    

    if(lc->bRenderToHardwareLayer)
    {
        return LAYER_RESULT_USE_OUTSIDE_BUFFER;
    }
    else
    {
        //* dequeue a buffer from the native window object, set it to a picture buffer wrapper.
        
        VideoPicture*     pPicture;
        int               i;
        int               err;
        ANativeWindowBuffer* pWindowBuf;
        void*                pDataBuf;
        
        if(lc->bLayerInitialized == 0)
        {
            if(SetLayerParam(lc) != 0)
            {
                loge("can not initialize layer.");
                return -1;
            }
            
            lc->bLayerInitialized = 1;
            
            //* now, we no need set 3D mode
            #if 0
            if(lc->eDisplay3DMode != DISPLAY_3D_MODE_2D || lc->ePicture3DMode != PICTURE_3D_MODE_NONE)
               LayerSetDisplay3DMode(l, lc->eDisplay3DMode);
            #endif
        }
        
        //* dequeue a buffer from the nativeWindow object.
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
            mapper.lock(pWindowBuf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &pDataBuf);
        }

        //* get a picture buffer wrapper.
        pPicture = NULL;
        for(i=0; i<32; i++)
        {
            if(lc->bBufferWrapperUsed[i] == 0)
            {
                lc->bBufferWrapperUsed[i] = 1;
                pPicture = &lc->bufferWrappers[i];
                break;
            }
        }
        
        if(i == 32)
        {
            loge("not enough picture buffer wrapper, shouldn't run here.");
            abort();
        }

#if((CONFIG_CHIP == OPTION_CHIP_1651) && (CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX))
		{
			//* set buffers to the picture buffer wrapper and return.
			memset(pPicture, 0, sizeof(VideoPicture));
			pPicture->ePixelFormat = lc->eReceivePixelFormat;
			pPicture->nWidth       = pWindowBuf->width;
			pPicture->nLineStride  = pWindowBuf->stride;
			pPicture->nHeight      = pWindowBuf->height;
			pPicture->pData0       = (char*)pDataBuf;
			pPicture->pData1       = pPicture->pData0 + (pWindowBuf->height * pWindowBuf->stride);
			pPicture->pData2       = pPicture->pData1 + (pWindowBuf->height * pWindowBuf->stride)/4;
		}
#else
        //* on chip-1673 and deinterlace, we should transform to phyAddr
        if(((CONFIG_CHIP == OPTION_CHIP_1673) && CONFIG_PRODUCT == OPTION_PRODUCT_PAD) 
            || lc->bDeinterlaceFlag == 1)
        {
            uintptr_t   nPhyaddress = -1;
            if(pWindowBuf->handle)
        	{

#if (CONFIG_CHIP != OPTION_CHIP_1680 && CONFIG_CHIP != OPTION_CHIP_1689)
        		IMG_native_handle_t* hnd = (IMG_native_handle_t*)(pWindowBuf->handle);
#else //for mali GPU
				private_handle_t* hnd = (private_handle_t *)(pWindowBuf->handle);
#endif
        		int fd = ion_open();
#if (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0)
        		ion_user_handle_t handle_ion;
#else
                struct ion_handle *handle_ion;
#endif

        		if(fd != -1)
        		{
#if (CONFIG_CHIP != OPTION_CHIP_1680 && CONFIG_CHIP != OPTION_CHIP_1689)
        			ion_import(fd, hnd->fd[0], &handle_ion);
#else
        			ion_import(fd, hnd->share_fd, &handle_ion);
#endif
        			nPhyaddress = ion_getphyadr(fd, handle_ion);
        			logv("++++++++phyaddress: %x\n", (unsigned int)nPhyaddress);
        			ion_close(fd);
        		}
        		else
        		{
        			loge("ion_open fail");
        			return -1;
        		}
        	}
        	else
        	{
        		loge("pWindowBuf->handle is null");
        		return -1;
        	}
            memset(pPicture, 0, sizeof(VideoPicture));
            pPicture->ePixelFormat = lc->eReceivePixelFormat;
            pPicture->nWidth       = pWindowBuf->width;
            pPicture->nLineStride  = pWindowBuf->stride;
            pPicture->nHeight      = pWindowBuf->height;
            pPicture->pData0       = (char*)nPhyaddress;
            pPicture->pData1       = (char*)(nPhyaddress + (pWindowBuf->height * pWindowBuf->stride));
            pPicture->pData2       = (char*)(nPhyaddress + (pWindowBuf->height * pWindowBuf->stride)*5/4);
            pPicture->phyYBufAddr  = nPhyaddress - PHY_OFFSET;
            pPicture->phyCBufAddr  = nPhyaddress + (pWindowBuf->height * pWindowBuf->stride) - PHY_OFFSET;
        }
        else
		{
            //* set buffers to the picture buffer wrapper and return.
            memset(pPicture, 0, sizeof(VideoPicture));
            pPicture->ePixelFormat = lc->eReceivePixelFormat;
            pPicture->nWidth       = pWindowBuf->width;
            pPicture->nLineStride  = pWindowBuf->stride;
            pPicture->nHeight      = pWindowBuf->height;
            pPicture->pData0       = (char*)pDataBuf;
            pPicture->pData1       = pPicture->pData0 + (pWindowBuf->height * pWindowBuf->stride);
            pPicture->pData2       = pPicture->pData1 + (pWindowBuf->height * pWindowBuf->stride)/4;
        }
#endif		

        *ppBuf = pPicture;
        lc->pWindowBufs[i] = pWindowBuf;
        return 0;
    }
}

void getVideoBufferInfo(int *nBufAddr, VideoPicture* pBuf0, VideoPicture* pBuf1)
{
    //HAL_PIXEL_FORMAT_AW_NV12		     = 0x101,
    //HAL_PIXEL_FORMAT_AW_MB420  		 = 0x102,
    //HAL_PIXEL_FORMAT_AW_MB411  	 	 = 0x103,
    //HAL_PIXEL_FORMAT_AW_MB422  		 = 0x104,
    //HAL_PIXEL_FORMAT_AW_YUV_PLANNER420 = 0x105,
	//HAL_PIXEL_FORMAT_AW_YVU_PLANNER420 = 0x106,
#if ((CONFIG_CHIP==OPTION_CHIP_1639 || CONFIG_CHIP==OPTION_CHIP_1673 || CONFIG_CHIP == OPTION_CHIP_1680) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))
    int temp;

    nBufAddr[0] = (int)MemAdapterGetPhysicAddress(pBuf0->pData0);
    nBufAddr[1] = (int)MemAdapterGetPhysicAddress(pBuf0->pData1);
    nBufAddr[2] = (int)MemAdapterGetPhysicAddress(pBuf0->pData2);

    if(pBuf1 != NULL)
    {
    	 nBufAddr[3] = (int)MemAdapterGetPhysicAddress(pBuf1->pData0);
    	 nBufAddr[4] = (int)MemAdapterGetPhysicAddress(pBuf1->pData1);
    	 nBufAddr[5] = (int)MemAdapterGetPhysicAddress(pBuf1->pData2);
    }

    switch(pBuf0->ePixelFormat)
    {
    	case PIXEL_FORMAT_YUV_MB32_420:
    	{
    	    nBufAddr[2] = 0;
    	    nBufAddr[5] = 0;
    		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_MB420;
    		break;
    	}
    	case PIXEL_FORMAT_YUV_MB32_422:
    	{
    		nBufAddr[2] = 0;
    		nBufAddr[5] = 0;
    		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_MB422;
        	break;
    	}
    	case PIXEL_FORMAT_NV12:
    	{
    		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_NV12;
    		break;
    	}
    	case PIXEL_FORMAT_NV21:
    	{
    		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_NV21;
    		break;
    	}
    	case PIXEL_FORMAT_YV12:
    	{
#if(CONFIG_CHIP==OPTION_CHIP_1639)            
            nBufAddr[6] = HAL_PIXEL_FORMAT_AW_YUV_PLANNER420;
    		temp = nBufAddr[1];
    		nBufAddr[1] = nBufAddr[2];
    		nBufAddr[2] = temp;

    		temp = nBufAddr[4];
    	    nBufAddr[4] = nBufAddr[5];
    	    nBufAddr[5] = temp;
#elif(CONFIG_CHIP==OPTION_CHIP_1673 || CONFIG_CHIP == OPTION_CHIP_1680)            
    		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_YVU_PLANNER420;
#endif
    		break;
    	}
    	case PIXEL_FORMAT_YUV_PLANER_420:
    	{
    		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_YUV_PLANNER420;
    		break;
    	}
    	default:
    	{
    		logd("error pixel format\n");
    	}
    }
#else
	CDX_PLAYER_UNUSE(nBufAddr);
	CDX_PLAYER_UNUSE(pBuf0);
	CDX_PLAYER_UNUSE(pBuf1);
#endif
    return;
}

int LayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;

	native_window_set_buffers_timestamp(lc->pNativeWindow, nPtsAbs);

	return 0;
}

static void updateKeepListBuffer(LayerCtrlContext* lc, buffer_handle_t handle, VideoPicture* pBuf0, VideoPicture* pBuf1)
{
    int i = 0;
    
    // 
    for(i = 0; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
    {
        if ((lc->pKeepPicNode[i].handle == handle)
            && (lc->pKeepPicNode[i].pPicture != NULL))
        {
            //* reture the old pic to ve
            lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pPicture);
            if (lc->pKeepPicNode[i].pSecondPictureOf3D != NULL)
            {
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pSecondPictureOf3D);
            }

            //* add the ve pic to keep-list
            lc->pKeepPicNode[i].pPicture = pBuf0;
            lc->pKeepPicNode[i].pSecondPictureOf3D = pBuf1;
            break;
        }
    }

    //* pBuf not in the list, add it
    if (i == NUM_OF_PICTURES_KEEP_IN_LIST)
    {
        for(i = 0; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
        {
            if ((lc->pKeepPicNode[i].handle == NULL)
                && (lc->pKeepPicNode[i].pPicture == NULL))
            {
                lc->pKeepPicNode[i].handle = handle;
                lc->pKeepPicNode[i].pPicture = pBuf0;
                lc->pKeepPicNode[i].pSecondPictureOf3D = pBuf1;
                break;
            }
        }
        //* add failed
        if (i == NUM_OF_PICTURES_KEEP_IN_LIST)
        {
            logw("add buffer to list failed, continue to add");
            //* return all buffer to ve
            for(i = 0; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
            {
                if (lc->pKeepPicNode[i].pPicture != NULL)
                {                
                    lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pPicture);
                    if (lc->pKeepPicNode[i].pSecondPictureOf3D != NULL)
                    {
                        lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pSecondPictureOf3D);
                    }
                    lc->pKeepPicNode[i].handle = NULL;
                    lc->pKeepPicNode[i].pPicture = NULL;
                    lc->pKeepPicNode[i].pSecondPictureOf3D = NULL;
                }
            }
            //* add in the head
            lc->pKeepPicNode[0].handle = handle;
            lc->pKeepPicNode[0].pPicture = pBuf0;
            lc->pKeepPicNode[0].pSecondPictureOf3D = pBuf1;
        }
    }
}


int LayerQueueBuffer(LayerCtrl* l, VideoPicture* pBuf, int bValid)
{
    LayerCtrlContext* lc;
    int               i;
    buffer_handle_t   handle = NULL;
    
    lc = (LayerCtrlContext*)l;

#if (DEBUG_DUMP_PIC)
    if (lc->picDump < 2)
    {
        FILE* fp1 = NULL;
        char path[512] = {0};
        
        loge("layer[%d] dump image hw(%d) size(%d * %d) , addr(%p/%p)", 
                    lc->picDump, lc->bRenderToHardwareLayer,  pBuf->nWidth, pBuf->nHeight,
                    pBuf->pData0, pBuf->pData1);

        sprintf(path, "/data/camera/layer.%d.dat", lc->picDump);
        fp1 = fopen(path, "wb");
//        MemAdapterFlushCache(pBuf->pData0, (pBuf->nWidth*pBuf->nHeight*3)/2);
        fwrite(pBuf->pData0, 1, (pBuf->nWidth*pBuf->nHeight*3)/2, fp1);
        fclose(fp1);

        lc->picDump++;
    }
#endif

    
    if(lc->bRenderToHardwareLayer)
    {
        libhwclayerpara_t hwcLayerPictureInfo;
        int                  err;
        ANativeWindowBuffer* pWindowBuf;
        int                 nBufAddr[7]={0};
        VideoPicture*       pPicture;
        void*               pDataBuf;
        
        if(bValid == 0)
        {
            if(pBuf != NULL)
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf);
            return 0;
        }
        
        if(lc->bLayerInitialized == 0)
        {
            if(SetLayerParam(lc) != 0)
            {
                loge("can not initialize layer.");
                return -1;
            }
            
            lc->bLayerInitialized = 1;

            //* now, we no need set 3D mode
            #if 0
            if(lc->eDisplay3DMode != DISPLAY_3D_MODE_2D || lc->ePicture3DMode != PICTURE_3D_MODE_NONE)
               LayerSetDisplay3DMode(l, lc->eDisplay3DMode);
            #endif
        }
    
        if(pBuf->nWidth != lc->nWidth ||
           pBuf->nHeight != lc->nHeight ||
           pBuf->ePixelFormat != lc->eReceivePixelFormat)
        {
            //* config the display hardware again.
            //* TODO.
        }
        
        //* set picture to display hardware.

        //* the 0 copy on A20 is different
#if(CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673 && CONFIG_CHIP != OPTION_CHIP_1680)
        setHwcLayerPictureInfo(&hwcLayerPictureInfo, pBuf, NULL);
        lc->pNativeWindow->perform(lc->pNativeWindow, 
                                   NATIVE_WINDOW_SETPARAMETER, 
                                   HWC_LAYER_SETFRAMEPARA, 
                                   (uintptr_t)(&hwcLayerPictureInfo));
        //* wait for new frame showed.
        if(lc->bLayerShowed == 1)
        {
        	int nCurFrameId;
            int nWaitTime;

            nWaitTime = 50000;  //* max frame interval is 1000/24fps = 41.67ms, here we wait 50ms for max.
            do
            {
            	nCurFrameId = lc->pNativeWindow->perform(lc->pNativeWindow,
                                                              NATIVE_WINDOW_SETPARAMETER,
                                                              HWC_LAYER_GETCURFRAMEPARA,
                                                              0);


                if(nCurFrameId == pBuf->nID)
                	break;
                else
                {
                	if(nWaitTime <= 0)
                	{
                		logv("check frame id fail, maybe something error with the HWC layer.");
                        break;
                	}
                	else
                	{
                		usleep(5000);
                        nWaitTime -= 5000;
                	}
                }
            }while(1);
        }
#else
        //* get a picture buffer wrapper.
        pPicture = NULL;
        for(i=0; i<32; i++)
        {
        	if(lc->bBufferWrapperUsed[i] == 0)
        	{
        		lc->bBufferWrapperUsed[i] = 1;
                 pPicture = &lc->bufferWrappers[i];
                 break;
        	}
        }

        if(i == 32)
        {
        	loge("not enough picture buffer wrapper, shouldn't run here.");
            abort();
        }

        //* dequeue a buffer from the nativeWindow object.
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
            mapper.lock(pWindowBuf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &pDataBuf);
			handle = pWindowBuf->handle;
        }

		if(lc->nOutputTypeChanged > 0 && lc->nOutputTypeChanged <= NUM_OF_PICTURES_KEEP_IN_LIST)
		{
			//logd("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx GPU->0 COPY, send black frame to GPU!!! %d", lc->nOutputTypeChanged);
			memset((char*)pDataBuf,0x10,(pWindowBuf->height * pWindowBuf->stride));
        	memset((char*)pDataBuf + pWindowBuf->height * pWindowBuf->stride,0x80,(pWindowBuf->height * pWindowBuf->stride)/2);
			lc->nOutputTypeChanged ++;
		}
		else if(lc->nOutputTypeChanged > NUM_OF_PICTURES_KEEP_IN_LIST)
		{
			//logd("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx send over!!");
			lc->nOutputTypeChanged = 0;
		}
		
        //memcpy(pPicture, pBuf, sizeof(VideoPicture));
        lc->pWindowBufs[i] = pWindowBuf;
#if(CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX)
        getVideoBufferInfo(nBufAddr, pBuf, NULL);
        lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO,	nBufAddr[0], nBufAddr[1],
        nBufAddr[2], nBufAddr[3], nBufAddr[4], nBufAddr[5], nBufAddr[6]);
        lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_DIT_INFO, !pBuf->bIsProgressive, pBuf->bTopFieldFirst);
#endif

        pWindowBuf = NULL;
        for(i=0; i<32; i++)
        {
        	if(pPicture == &lc->bufferWrappers[i])
        	{
        		pWindowBuf = lc->pWindowBufs[i];
                lc->bBufferWrapperUsed[i] = 0;
                lc->pWindowBufs[i]        = NULL;
                break;
        	}
        }

        if(i == 32 || pWindowBuf == NULL)
        {
        	loge("enqueue an invalid buffer.");
            abort();
        }
        //* unlock the buffer.
        {
        	GraphicBufferMapper& mapper = GraphicBufferMapper::get();
            mapper.unlock(pWindowBuf->handle);
        }

        if(bValid)
        	lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
        else
            lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
#endif

        //* free last picture of last video stream in case gConfigHoldLastPicture is set.
        if(gLastPicture != NULL)
        {
            logv("xxxx free gLastPicture, pData0 = %p", gLastPicture->pData0);
            FreePictureBuffer(gLastPicture);
            gLastPicture = NULL;
        }

		updateKeepListBuffer(lc, handle, pBuf, NULL);
    }
    else
    {
		/*
		 * queue to GPU here
		 */

        int                  err;
        ANativeWindowBuffer* pWindowBuf;
        
        pWindowBuf = NULL;
        for(i=0; i<32; i++)
        {
            if(pBuf == &lc->bufferWrappers[i])
            {
                pWindowBuf = lc->pWindowBufs[i];
                lc->bBufferWrapperUsed[i] = 0;
                lc->pWindowBufs[i]        = NULL;
                break;
            }
        }
        
        if(i == 32 || pWindowBuf == NULL)
        {
            loge("enqueue an invalid buffer.");
            abort();
        }
        
        //* unlock the buffer.
        {
            GraphicBufferMapper& mapper = GraphicBufferMapper::get();
            mapper.unlock(pWindowBuf->handle);
        }

#if ((CONFIG_CHIP==OPTION_CHIP_1639 || CONFIG_CHIP==OPTION_CHIP_1673 || CONFIG_CHIP == OPTION_CHIP_1680) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX)) 
         //* On A80-box, we must set comman NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO
         //* to notify GPU that this is 'cpy to gpu', not 'o-cpy' 
         int nBufParam[7] = {0};
         lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO,nBufParam[0], nBufParam[1],
         nBufParam[2], nBufParam[3], nBufParam[4], nBufParam[5], nBufParam[6]);
#endif
        if(bValid)
            lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
        else
            lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    }

	if(lc->bFirstFrameIsShowed != 1)
	{
		lc->bFirstFrameIsShowed = 1;
	}

	return 0;
}


int LayerDequeue3DBuffer(LayerCtrl* l, VideoPicture** ppBuf0, VideoPicture** ppBuf1)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    *ppBuf0 = NULL;
    *ppBuf1 = NULL;
    
    if(lc->bRenderToHardwareLayer)
    {
        return LAYER_RESULT_USE_OUTSIDE_BUFFER;
    }
    else
    {
        if(lc->bLayerInitialized == 0)
        {
            if(SetLayerParam(lc) != 0)
            {
                loge("can not initialize layer.");
                return -1;
            }
            
            lc->bLayerInitialized = 1;

            //* now, we no need set 3D mode
            #if 0
            if(lc->eDisplay3DMode != DISPLAY_3D_MODE_2D || lc->ePicture3DMode != PICTURE_3D_MODE_NONE)
               LayerSetDisplay3DMode(l, lc->eDisplay3DMode);
            #endif
        }
        
        logw("can not render 3D picture(two seperated pictures) when not \
                rendering to hardware layer.");
        return LayerDequeueBuffer(l, ppBuf0, 1);
    }
}


int LayerQueue3DBuffer(LayerCtrl* l, VideoPicture* pBuf0, VideoPicture* pBuf1, int bValid)
{
    LayerCtrlContext* lc;
    int               i;
    buffer_handle_t   handle = NULL;
    
    lc = (LayerCtrlContext*)l;
    
    if(lc->bRenderToHardwareLayer)
    {
        libhwclayerpara_t hwcLayerPictureInfo;
        int               nBufAddr[7]={0};
        VideoPicture*     pPicture;
        void*             pDataBuf;
        int               err;
        ANativeWindowBuffer* pWindowBuf;
        
        if(bValid == 0)
        {
            if(pBuf0 != NULL)
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf0);
            if(pBuf1 != NULL)
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf1);
            return 0;
        }
    
        if(lc->bLayerInitialized == 0)
        {
            if(SetLayerParam(lc) != 0)
            {
                loge("can not initialize layer.");
                return -1;
            }
            
            lc->bLayerInitialized = 1;

            //* now, we no need set 3D mode
            #if 0
            if(lc->eDisplay3DMode != DISPLAY_3D_MODE_2D || lc->ePicture3DMode != PICTURE_3D_MODE_NONE)
               LayerSetDisplay3DMode(l, lc->eDisplay3DMode);
            #endif
        }
    
        if(pBuf0->nWidth != lc->nWidth ||
           pBuf0->nHeight != lc->nHeight ||
           pBuf0->ePixelFormat != lc->eReceivePixelFormat)
        {
            //* config the display hardware again.
            //* TODO.
        }

		//* the 0 copy on A20 is different
#if(CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673 && CONFIG_CHIP != OPTION_CHIP_1680)
        //* set picture to display hardware.
        setHwcLayerPictureInfo(&hwcLayerPictureInfo, pBuf0, pBuf1);
        lc->pNativeWindow->perform(lc->pNativeWindow, 
                                   NATIVE_WINDOW_SETPARAMETER, 
                                   HWC_LAYER_SETFRAMEPARA, 
                                   (uintptr_t)(&hwcLayerPictureInfo));
        
        //* wait for new frame showed.
        if(lc->bLayerShowed == 1)
        {
            int nCurFrameId;
            int nWaitTime;
            
            nWaitTime = 50000;  //* max frame interval is 1000/24fps = 41.67ms, here we wait 50ms for max.
            do
            {
                nCurFrameId = lc->pNativeWindow->perform(lc->pNativeWindow, 
                                                         NATIVE_WINDOW_SETPARAMETER, 
                                                         HWC_LAYER_GETCURFRAMEPARA, 
                                                         0);
                if(nCurFrameId == pBuf0->nID)
                    break;
                else
                {
                    if(nWaitTime <= 0)
                    {
                        loge("check frame id fail, maybe something error with the HWC layer.");
                        break;
                    }
                    else
                    {
                        usleep(5000);
                        nWaitTime -= 5000;
                    }
                }
            }while(1);
        }
#else
        //* get a picture buffer wrapper.
         pPicture = NULL;
         for(i=0; i<32; i++)
         {
        	 if(lc->bBufferWrapperUsed[i] == 0)
             {
        		 lc->bBufferWrapperUsed[i] = 1;
        		 pPicture = &lc->bufferWrappers[i];
        		 break;
             }
         }

         if(i == 32)
         {
        	 loge("not enough picture buffer wrapper, shouldn't run here.");
             abort();
         }

         //* dequeue a buffer from the nativeWindow object.
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
             mapper.lock(pWindowBuf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &pDataBuf);
             handle = pWindowBuf->handle;
         }

         lc->pWindowBufs[i] = pWindowBuf;

#if(CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX)
         getVideoBufferInfo(nBufAddr, pBuf0, pBuf1);
         lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO,nBufAddr[0], nBufAddr[1],
        		 nBufAddr[2], nBufAddr[3], nBufAddr[4],nBufAddr[5], nBufAddr[6]);
         lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_DIT_INFO, !pBuf0->bIsProgressive, pBuf0->bTopFieldFirst);
#endif

         pWindowBuf = NULL;
         for(i=0; i<32; i++)
         {
        	 if(pPicture == &lc->bufferWrappers[i])
        	 {
               		pWindowBuf = lc->pWindowBufs[i];
                    lc->bBufferWrapperUsed[i] = 0;
                    lc->pWindowBufs[i]        = NULL;
                    break;
        	 }
         }

         if(i == 32 || pWindowBuf == NULL)
         {
        	 loge("enqueue an invalid buffer.");
             abort();
         }
         //* unlock the buffer.
         {
        	 GraphicBufferMapper& mapper = GraphicBufferMapper::get();
        	 mapper.unlock(pWindowBuf->handle);
         }

         if(bValid)
        	 lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
         else
        	 lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
#endif
        
		updateKeepListBuffer(lc, handle, pBuf0, pBuf1);
		 
		if(lc->bFirstFrameIsShowed != 1)
		{
			lc->bFirstFrameIsShowed = 1;
		}
		
        return 0;
    }
    else
    {
        logw("can not render 3D picture(two seperated pictures) when not \
                rendering to hardware layer.");
        if(bValid)
            return LayerQueueBuffer(l, pBuf0, 1);
        else
            return LayerQueueBuffer(l, pBuf0, 0);
    }
}


static int SetLayerParam(LayerCtrlContext* lc)
{
    if(lc->bRenderToHardwareLayer)
    {
        int                   pixelFormat;
        android_native_rect_t crop;
        
        //* close the layer first, otherwise, in case when last frame is kept showing,
        //* the picture showed will not valid because parameters changed.
        logv("Set layer param.");
        logv("temporally close the HWC layer when parameter changed.");
    	lc->bLayerShowed = 0;
        lc->pNativeWindow->perform(lc->pNativeWindow,
                                       NATIVE_WINDOW_SETPARAMETER,
                                       HWC_LAYER_SHOW,
                                       0);

        pixelFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        
#if((CONFIG_CHIP == OPTION_CHIP_1673 || CONFIG_CHIP == OPTION_CHIP_1680) && CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX)
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
#elif(CONFIG_CHIP!=OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673 && CONFIG_CHIP != OPTION_CHIP_1680)      //* display system do not support MB32.
        switch(lc->eReceivePixelFormat)
        {
            case PIXEL_FORMAT_YUV_PLANER_420:
            case PIXEL_FORMAT_YV12:             //* why YV12 use this pixel format.
                pixelFormat = HWC_FORMAT_YUV420PLANAR;
                break;
            case PIXEL_FORMAT_YUV_MB32_420:
                pixelFormat = HWC_FORMAT_MBYUV420;
                break;
            case PIXEL_FORMAT_YUV_MB32_422:
                pixelFormat = HWC_FORMAT_MBYUV422;
                break;
            case PIXEL_FORMAT_YUV_PLANER_422:
                pixelFormat = HWC_FORMAT_MBYUV422;
                break;
            default:
            {
                loge("unsupported pixel format.");
                return -1;
                break;
            }
        }
#endif
        
        native_window_set_usage(lc->pNativeWindow,
                                GRALLOC_USAGE_SW_READ_NEVER  |
                                GRALLOC_USAGE_SW_WRITE_OFTEN |
                                GRALLOC_USAGE_HW_TEXTURE     |
                                GRALLOC_USAGE_EXTERNAL_DISP);
                                
        if(pixelFormat == HWC_FORMAT_YUV420PLANAR)
        {
            native_window_set_buffers_geometry(lc->pNativeWindow,
                                               lc->nWidth,
                                               lc->nHeight,
                                               pixelFormat);
        }
        else
        {
            native_window_set_scaling_mode(lc->pNativeWindow, 
                                           NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
           
            native_window_set_buffers_geometry(lc->pNativeWindow,
                                                 lc->nWidth,
                                                 lc->nHeight,
                                                 pixelFormat);    //* screen id = 0.
        }
        
        crop.left   = lc->nLeftOff;
        crop.right  = lc->nLeftOff + lc->nDisplayWidth;
        crop.top    = lc->nTopOff;
        crop.bottom = lc->nTopOff + lc->nDisplayHeight;
		lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, (uintptr_t)&crop);
        
        lc->bLayerShowed = 0;
    }
    else
    {
        int          pixelFormat;
        unsigned int nGpuBufWidth;
        unsigned int nGpuBufHeight;
        Rect         crop;
        
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

        //*on box , we should add the usage of GRALLOC_USAGE_SW_WRITE_OFTEN
#if (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX)        
        native_window_set_usage(lc->pNativeWindow,
                            GRALLOC_USAGE_SW_READ_NEVER     |
                            GRALLOC_USAGE_SW_WRITE_OFTEN    | 
                            GRALLOC_USAGE_HW_TEXTURE        | 
                            GRALLOC_USAGE_EXTERNAL_DISP);
#else
        native_window_set_usage(lc->pNativeWindow,
                            GRALLOC_USAGE_SW_READ_NEVER     |
                            //GRALLOC_USAGE_SW_WRITE_OFTEN    | 
                            GRALLOC_USAGE_HW_TEXTURE        | 
                            GRALLOC_USAGE_EXTERNAL_DISP);
#endif

        native_window_set_scaling_mode(lc->pNativeWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

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
        
        nGpuBufWidth = lc->nWidth;  //* restore nGpuBufWidth to mWidth;
    
        native_window_set_buffers_geometry(lc->pNativeWindow, nGpuBufWidth, nGpuBufHeight, pixelFormat);
        
        crop.left   = lc->nLeftOff;
        crop.top    = lc->nTopOff;
        crop.right  = lc->nLeftOff + lc->nDisplayWidth;
        crop.bottom = lc->nTopOff + lc->nDisplayHeight;
        lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, &crop);
    }
    /* native_window_set_buffer_count() will free buffers insides, so do not do it again when seek */
    if (lc->bFirstFrameIsShowed == 0)
    {
        /*
         * set 5 frames when Interlaced, for smooth play in high fps,
         * restore to 3 frames when Progressive, because decoder(VE) assumed it.
         * if used 5 frames in Progressive, DE buffer will changed by VE.
         */
        //*if we set buffer count as 5, will cause some problem,
        // we can ask hankewei what is the problem. so we set it to 3.
        if (lc->bDeinterlaceFlag == DE_INTERLACE_HW)
            native_window_set_buffer_count(lc->pNativeWindow, 3);
        else
            native_window_set_buffer_count(lc->pNativeWindow, 3);
    }
    return 0;
}

static void setHwcLayerPictureInfo(libhwclayerpara_t* pHwcLayerPictureInfo,
                                   VideoPicture*      pPicture,
                                   VideoPicture*      pSecondPictureOf3D)
{
    memset(pHwcLayerPictureInfo, 0, sizeof(libhwclayerpara_t));
    
    if(pSecondPictureOf3D == NULL)
    {
        pHwcLayerPictureInfo->number          = pPicture->nID;
        pHwcLayerPictureInfo->bProgressiveSrc = pPicture->bIsProgressive;
        pHwcLayerPictureInfo->bTopFieldFirst  = pPicture->bTopFieldFirst;
        pHwcLayerPictureInfo->flag_addr       = (unsigned long)MemAdapterGetPhysicAddress(pPicture->pMafData);
        pHwcLayerPictureInfo->flag_stride     = pPicture->nMafFlagStride;
        pHwcLayerPictureInfo->maf_valid       = pPicture->bMafValid;
        pHwcLayerPictureInfo->pre_frame_valid = pPicture->bPreFrmValid;
        pHwcLayerPictureInfo->top_y           = (unsigned long)MemAdapterGetPhysicAddress(pPicture->pData0);
        pHwcLayerPictureInfo->top_c           = (unsigned long)MemAdapterGetPhysicAddress(pPicture->pData1);
        pHwcLayerPictureInfo->bottom_y        = (unsigned long)MemAdapterGetPhysicAddress(pPicture->pData2);
        pHwcLayerPictureInfo->bottom_c        = (unsigned long)MemAdapterGetPhysicAddress(pPicture->pData3);
    }
    else
    {
        pHwcLayerPictureInfo->number          = pPicture->nID;
        pHwcLayerPictureInfo->bProgressiveSrc = pPicture->bIsProgressive;
        pHwcLayerPictureInfo->bTopFieldFirst  = pPicture->bTopFieldFirst;
        pHwcLayerPictureInfo->flag_addr       = (unsigned long)MemAdapterGetPhysicAddress(pPicture->pMafData);
        pHwcLayerPictureInfo->flag_stride     = pPicture->nMafFlagStride;
        pHwcLayerPictureInfo->maf_valid       = pPicture->bMafValid;
        pHwcLayerPictureInfo->pre_frame_valid = pPicture->bPreFrmValid;
        pHwcLayerPictureInfo->top_y           = (unsigned long)MemAdapterGetPhysicAddress(pPicture->pData0);
        pHwcLayerPictureInfo->top_c           = (unsigned long)MemAdapterGetPhysicAddress(pPicture->pData1);
        pHwcLayerPictureInfo->bottom_y        = (unsigned long)MemAdapterGetPhysicAddress(pSecondPictureOf3D->pData0);
        pHwcLayerPictureInfo->bottom_c        = (unsigned long)MemAdapterGetPhysicAddress(pSecondPictureOf3D->pData1);
    }
    
    return;
}

int LayerCtrlShowVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;
    logd("xxxx show video, current show flag = %d", lc->bLayerShowed);
    if(lc->bLayerShowed == 0)
    {
    	lc->bLayerShowed = 1;
        lc->pNativeWindow->perform(lc->pNativeWindow,
                                   NATIVE_WINDOW_SETPARAMETER,
                                   HWC_LAYER_SHOW,
                                   1);
    }
    return 0;
}


int LayerCtrlHideVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;
    logd("xxxx hide video, current show flag = %d", lc->bLayerShowed);
    if(lc->bLayerShowed == 1)
    {
    	lc->bLayerShowed = 0;
        lc->pNativeWindow->perform(lc->pNativeWindow,
                                       NATIVE_WINDOW_SETPARAMETER,
                                       HWC_LAYER_SHOW,
                                       0);
    }
    return 0;
}


int LayerCtrlIsVideoShow(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    return lc->bLayerShowed;
}


int LayerCtrlHoldLastPicture(LayerCtrl* l, int bHold)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;

    logd("LayerCtrlHoldLastPicture, bHold = %d", bHold);
    if(bHold == 0)
    {
        if(gLastPicture != NULL)
        {
            FreePictureBuffer(gLastPicture);
            gLastPicture = NULL;
        }
        return 0;
    }

	lc->keepLastFrameFlag = true;

    //* just on tvbox have the 0-copy
#if(CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX)
    if(lc->bRenderToHardwareLayer)
    {
        VideoPicture* lastPicture = NULL;

        if (lc->pKeepPicNode[0].pPicture != NULL)
        {
        	lastPicture = lc->pKeepPicNode[0].pPicture;
        }
		//* find the latest pic which the pts is laragest
        for (int i = 1; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
        {
        	if ((lc->pKeepPicNode[i].pPicture != NULL)
				&& (lastPicture->nPts < lc->pKeepPicNode[i].pPicture->nPts))
        	{
        		lastPicture = lc->pKeepPicNode[i].pPicture;
        	}
        }
            
        if(lastPicture != NULL)
        {
            if(gLastPicture != NULL)
                FreePictureBuffer(gLastPicture);

#if(CONFIG_CHIP == OPTION_CHIP_1651)
            gLastPicture = AllocatePictureBuffer(lastPicture->nWidth,
                                                 lastPicture->nHeight,
                                                 lastPicture->nLineStride,
                                                 lastPicture->ePixelFormat);
            logd("width = %d, height = %d, pdata0 = %p", 
                gLastPicture->nWidth, gLastPicture->nHeight, gLastPicture->pData0);
            if(gLastPicture != NULL)
            {
				//* the 0 copy on A20 is different          
                libhwclayerpara_t hwcLayerPictureInfo;
                
                gLastPicture->nID = 0xa5a5a5a5;
                RotatePicture(lastPicture, gLastPicture, 0, lc->nGpuYAlign, lc->nGpuCAlign);
                
                //* set picture to display hardware.
                setHwcLayerPictureInfo(&hwcLayerPictureInfo, gLastPicture, NULL);
                lc->pNativeWindow->perform(lc->pNativeWindow, 
                                           NATIVE_WINDOW_SETPARAMETER, 
                                           HWC_LAYER_SETFRAMEPARA, 
                                           (unsigned int)(&hwcLayerPictureInfo));
                                           
                //* wait for new frame showed.
                if(lc->bLayerShowed == 1)
                {
                    int nCurFrameId;
                    int nWaitTime;
                    
                    nWaitTime = 50000;  //* max frame interval is 1000/24fps = 41.67ms, here we wait 50ms for max.
                    do
                    {
                        nCurFrameId = lc->pNativeWindow->perform(lc->pNativeWindow, 
                                                                 NATIVE_WINDOW_SETPARAMETER, 
                                                                 HWC_LAYER_GETCURFRAMEPARA, 
                                                                 0);
                        if(nCurFrameId == gLastPicture->nID)
                            break;
                        else
                        {
                            if(nWaitTime <= 0)
                            {
                                loge("check frame id fail, maybe something error with the HWC layer.");
                                break;
                            }
                            else
                            {
                                usleep(5000);
                                nWaitTime -= 5000;
                            }
                        }
                    }while(1);
                }
            }
#else

            if(lc->nGpuYAlign == 32)
            {
    			gLastPicture = AllocatePictureBuffer((lastPicture->nWidth+31)&~31,
                                                     lastPicture->nHeight,
                                                     lastPicture->nLineStride,
                                                     lastPicture->ePixelFormat);
            }
            else if(lc->nGpuYAlign == 16)
            {
                gLastPicture = AllocatePictureBuffer((lastPicture->nWidth+15)&~15,
                                                 lastPicture->nHeight,
                                                 lastPicture->nLineStride,
                                                 lastPicture->ePixelFormat);
            }
            else
            {
                loge("the nGpuYAlign[%d] is not surpport!",lc->nGpuYAlign);
                return -1;
            }
            
			RotatePicture(lastPicture, gLastPicture, 0, lc->nGpuYAlign, lc->nGpuCAlign);
            
            logd("width = %d, height = %d, pdata0 = %p", 
                gLastPicture->nWidth, gLastPicture->nHeight, gLastPicture->pData0);
            if(gLastPicture != NULL)
            {
                int                  err;
                ANativeWindowBuffer* pWindowBuf = NULL;
                int                  nBufAddr[7]={0};
                VideoPicture*        pPicture   = NULL;
                void*                pDataBuf   = NULL;

				if (gLastPicture->ePixelFormat != lc->eReceivePixelFormat)
				{
					logd("picture format=0x%x, layer format=0x%x", gLastPicture->ePixelFormat, lc->eReceivePixelFormat);
					lc->bRenderToHardwareLayer = 0;
					lc->eReceivePixelFormat = (enum EPIXELFORMAT)gLastPicture->ePixelFormat;
					SetLayerParam(lc);
				}

				for(int k = 0;k < NUM_OF_PICTURES_KEEP_IN_LIST;k++)
				{
	                int i;
	                //* get a picture buffer wrapper.
	                for(i=0; i<32; i++)
	                {
	                	if(lc->bBufferWrapperUsed[i] == 0)
	                	{
	                		lc->bBufferWrapperUsed[i] = 1;
	                         pPicture = &lc->bufferWrappers[i];
	                         break;
	                	}
	                }

	                if(i == 32)
	                {
	                	loge("not enough picture buffer wrapper, shouldn't run here.");
	                    abort();
	                }

	                //* dequeue a buffer from the nativeWindow object.
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
	                    mapper.lock(pWindowBuf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &pDataBuf);
	                }

	                //memcpy(pPicture, pBuf, sizeof(VideoPicture));
	                lc->pWindowBufs[i] = pWindowBuf;
	                //getVideoBufferInfo(nBufAddr, gLastPicture, NULL);
#if 0
	                int y_size = pWindowBuf->height * pWindowBuf->stride;
	                if (gLastPicture->ePixelFormat == PIXEL_FORMAT_YV12)
	                {
	                    memcpy((char*)pDataBuf,gLastPicture->pData0,(pWindowBuf->height * pWindowBuf->stride));
	                    memcpy((char*)pDataBuf+y_size,gLastPicture->pData1,(pWindowBuf->height * pWindowBuf->stride/4));
	                    memcpy((char*)pDataBuf+y_size*5/4,gLastPicture->pData2,(pWindowBuf->height * pWindowBuf->stride/4));
	                }
	                else if (gLastPicture->ePixelFormat == PIXEL_FORMAT_NV21)
	                {
	                    memcpy((char*)pDataBuf,gLastPicture->pData0,(pWindowBuf->height * pWindowBuf->stride));
	                    memcpy((char*)pDataBuf+y_size,gLastPicture->pData1,(pWindowBuf->height * pWindowBuf->stride/2));
	                }
	                else
	                {
	                    memcpy((char*)pDataBuf,gLastPicture->pData0,(pWindowBuf->height * pWindowBuf->stride * 3 / 2));
	                }
#endif
	                VideoPicture pPictureTmp;
	                pPictureTmp.ePixelFormat = lc->eReceivePixelFormat;
	                pPictureTmp.nWidth		 = pWindowBuf->width;
	                pPictureTmp.nLineStride  = pWindowBuf->stride;
	                pPictureTmp.nHeight 	 = pWindowBuf->height;
	                pPictureTmp.pData0		 = (char*)pDataBuf;
	                pPictureTmp.pData1		 = pPictureTmp.pData0 + (pWindowBuf->height * pWindowBuf->stride);
	                pPictureTmp.pData2		 = pPictureTmp.pData1 + (pWindowBuf->height * pWindowBuf->stride)/4;
	                RotatePicture(lastPicture, &pPictureTmp, 0, lc->nGpuYAlign, lc->nGpuCAlign);

			        int nBufAddr[7] = {0};
			        //nBufAddr[6] = HAL_PIXEL_FORMAT_AW_NV12;
			        nBufAddr[6] = HAL_PIXEL_FORMAT_AW_FORCE_GPU;
	                lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO,	nBufAddr[0], nBufAddr[1],
	                nBufAddr[2], nBufAddr[3], nBufAddr[4], nBufAddr[5], nBufAddr[6]);
	                //lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_DIT_INFO, !gLastPicture->bIsProgressive, gLastPicture->bTopFieldFirst);
	                pWindowBuf = NULL;
	                for(i=0; i<32; i++)
	                {
	                	if(pPicture == &lc->bufferWrappers[i])
	                	{
	                		pWindowBuf = lc->pWindowBufs[i];
	                        lc->bBufferWrapperUsed[i] = 0;
	                        lc->pWindowBufs[i]        = NULL;
	                        break;
	                	}
	                }

	                if(i == 32 || pWindowBuf == NULL)
	                {
	                	loge("enqueue an invalid buffer.");
	                    abort();
	                }
	                //* unlock the buffer.
	                {
	                	GraphicBufferMapper& mapper = GraphicBufferMapper::get();
	                    mapper.unlock(pWindowBuf->handle);
	                }

	            	lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
				}
            }
#endif			
        }
    }
#endif    
    return 0;
}

static int SendThreeBlackFrameToGpu(LayerCtrlContext* lc)
{
    logd("SendThreeBlackFrameToGpu()");
    
    ANativeWindowBuffer* pWindowBuf;
    void*                pDataBuf;
    int                  i;
    int                  err;

    //* it just work on A80-box and H8         
#if((CONFIG_CHIP==OPTION_CHIP_1639 || CONFIG_CHIP==OPTION_CHIP_1673 || CONFIG_CHIP == OPTION_CHIP_1680) && (CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX))    
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
            mapper.lock(pWindowBuf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &pDataBuf);
        }

        memset((char*)pDataBuf,0x10,(pWindowBuf->height * pWindowBuf->stride));
        memset((char*)pDataBuf + pWindowBuf->height * pWindowBuf->stride,0x80,(pWindowBuf->height * pWindowBuf->stride)/2);
        
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
#else
	CDX_PLAYER_UNUSE(lc);
#endif
    return 0;
}

int LayerSetPicture3DMode(LayerCtrl* l, enum EPICTURE3DMODE ePicture3DMode)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(ePicture3DMode);
    logw("not implement the function LayerSetPicture3DMode");
    return -1;
}

enum EPICTURE3DMODE LayerGetPicture3DMode(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function LayerGetPicture3DMode");
    return PICTURE_3D_MODE_NONE;
}

int LayerSetDisplay3DMode(LayerCtrl* l, enum EDISPLAY3DMODE eDisplay3DMode)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(eDisplay3DMode);
    logw("not implement the function LayerSetDisplay3DMode");
    return -1;
}

enum EDISPLAY3DMODE LayerGetDisplay3DMode(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function LayerGetDisplay3DMode");
    return DISPLAY_3D_MODE_2D;
}

int LayerSetBufferCount(LayerCtrl* l, int nBufferCount)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(nBufferCount);
    logw("not implement the function LayerSetBufferCount");
    return -1;
}

int LayerSetVideoWithTwoStreamFlag(LayerCtrl* l, int bVideoWithTwoStreamFlag)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(bVideoWithTwoStreamFlag);
    logw("not implement the function LayerSetVideoWithTwoStreamFlag");
    return -1;
}

int LayerSetIsSoftDecoderFlag(LayerCtrl* l, int bIsSoftDecoderFlag)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(bIsSoftDecoderFlag);
    logw("not implement the function LayerSetIsSoftDecoderFlag");
    return -1;
}

void LayerResetNativeWindow(LayerCtrl* l,void* pNativeWindow)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(pNativeWindow);
    logw("not implement the function LayerResetNativeWindow");
    return ;
}

int LayerReleaseBuffer(LayerCtrl* l,VideoPicture* pPicture)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(pPicture);
    logw("not implement the function LayerReleaseBuffer");
    return -1;
}

VideoPicture* LayerGetPicNode(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function LayerGetPicNode");
    return NULL;
}

int LayerGetAddedPicturesCount(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function LayerGetAddedPicturesCount");
    return -1;
}

int LayerGetDisplayFPS(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function LayerGetDisplayFPS");
    return -1;
}

LayerControlOpsT mLayerControlOps = 
{
    LayerInit:                       LayerInit                      ,
    LayerRelease:                    LayerRelease                   ,
    LayerSetExpectPixelFormat:       LayerSetExpectPixelFormat      ,
    LayerGetPixelFormat:             LayerGetPixelFormat            ,
    LayerSetPictureSize:             LayerSetPictureSize            ,
    LayerSetDisplayRegion:           LayerSetDisplayRegion          ,
    LayerSetBufferTimeStamp:         LayerSetBufferTimeStamp        ,
    LayerDequeueBuffer:              LayerDequeueBuffer             ,
    LayerQueueBuffer:                LayerQueueBuffer               ,
    LayerCtrlHideVideo:              LayerCtrlHideVideo             ,
    LayerCtrlShowVideo:              LayerCtrlShowVideo             ,
    LayerCtrlIsVideoShow:		     LayerCtrlIsVideoShow           ,
    LayerCtrlHoldLastPicture:        LayerCtrlHoldLastPicture       ,
    LayerSetRenderToHardwareFlag:    LayerSetRenderToHardwareFlag   ,
    LayerSetDeinterlaceFlag:         LayerSetDeinterlaceFlag        ,
    LayerSetPicture3DMode:           LayerSetPicture3DMode          ,
    LayerGetPicture3DMode:           LayerGetPicture3DMode          ,
    LayerSetDisplay3DMode:           LayerSetDisplay3DMode          ,
    LayerGetDisplay3DMode:           LayerGetDisplay3DMode          ,
    LayerDequeue3DBuffer:            LayerDequeue3DBuffer           ,
    LayerQueue3DBuffer:              LayerQueue3DBuffer             ,
    LayerGetRotationAngle:           LayerGetRotationAngle          ,
    LayerSetCallback:                LayerSetCallback               ,
    LayerSetBufferCount:             LayerSetBufferCount            ,
    LayerSetVideoWithTwoStreamFlag:  LayerSetVideoWithTwoStreamFlag ,        
    LayerSetIsSoftDecoderFlag:       LayerSetIsSoftDecoderFlag      ,
    LayerResetNativeWindow:          LayerResetNativeWindow         ,
    LayerReleaseBuffer:              LayerReleaseBuffer             ,
    LayerGetPicNode:                 LayerGetPicNode                ,
    LayerGetAddedPicturesCount:      LayerGetAddedPicturesCount     ,
    LayerGetDisplayFPS:              LayerGetDisplayFPS             
};
