#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "config.h"
#include "layerControl.h"
#include "log.h"
#include "player_i.h"


#define SAVE_PIC (0)

#include "1623/drv_display.h"


static VideoPicture* gLastPicture = NULL;

#define BUF_MANAGE (0)


int LayerCtrlHideVideo(LayerCtrl* l);

typedef struct VPictureNode_t VPictureNode;
struct VPictureNode_t
{
    VideoPicture* pPicture;
    VideoPicture* pSecondPictureOf3D;
    VPictureNode* pNext;
    int           bUsed;
};


typedef struct LayerCtrlContext
{  
    int                  nScreenWidth;
    int                  nScreenHeight;
    LayerCtlCallback       callback;
    void*                pUserData;

    //* use when render derect to hardware layer.
    VPictureNode*        pPictureListHead;
    VPictureNode         picNodes[32];
    
}LayerCtrlContext;

static int SetLayerParam(LayerCtrlContext* lc);
static void setHwcLayerPictureInfo(LayerCtrlContext* lc,
                                   disp_layer_info*  pLayerInfo, 
                                   VideoPicture*     pPicture,
                                   VideoPicture*     pSecondPictureOf3D);


LayerCtrl* LayerInit(void* pNativeWindow, int bProtectedFlag)
{
    unsigned long     args[4];
    LayerCtrlContext* lc;
    
    logv("LayerInit.");
    
    lc = (LayerCtrlContext*)malloc(sizeof(LayerCtrlContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerCtrlContext));

    
    return (LayerCtrl*)lc;
}


void LayerRelease(LayerCtrl* l, int bKeepPictureOnScreen)
{
    LayerCtrlContext* lc;
    VPictureNode*     nodePtr;
    unsigned long     args[4];
	
    lc = (LayerCtrlContext*)l;
    
    logv("Layer release");
    
    //* return pictures.
    while(lc->pPictureListHead != NULL)
    {
        nodePtr = lc->pPictureListHead;
        lc->pPictureListHead = lc->pPictureListHead->pNext;
        lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)nodePtr->pPicture);
    }
    
    
    free(lc);   
}


int LayerSetExpectPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    return 0;
}


enum EPIXELFORMAT LayerGetPixelFormat(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    return PIXEL_FORMAT_DEFAULT;
}


int LayerSetPictureSize(LayerCtrl* l, int nWidth, int nHeight)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer set picture size, width = %d, height = %d", nWidth, nHeight);
    
    return 0;
}


int LayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff, int nDisplayWidth, int nDisplayHeight)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, displayHeight = %d",
        nLeftOff, nTopOff, nDisplayWidth, nDisplayHeight);
    
    return -1;
}


int LayerSetPicture3DMode(LayerCtrl* l, enum EPICTURE3DMODE ePicture3DMode)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer set picture 3d mode, mode = %d", (int)ePicture3DMode);
      
    return 0;
}


enum EPICTURE3DMODE LayerGetPicture3DMode(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer get picture 3d mode, mode = %d", (int)lc->ePicture3DMode);
    
    return PICTURE_3D_MODE_NONE;
}


int LayerSetDisplay3DMode(LayerCtrl* l, enum EDISPLAY3DMODE eDisplay3DMode)
{
    LayerCtrlContext* lc;
    disp_layer_info   layerInfo;
    unsigned long     args[4];
    int               err;
    
    lc = (LayerCtrlContext*)l;
    

    logw("set 3d mode fail to hardware layer.");
    return -1;

}


enum EDISPLAY3DMODE LayerGetDisplay3DMode(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logv("Layer get display 3d mode, mode = %d", (int)lc->eDisplay3DMode);
    
    return DISPLAY_3D_MODE_2D;
}


int LayerSetCallback(LayerCtrl* l, LayerCtlCallback callback, void* pUserData)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    lc->callback  = callback;
    lc->pUserData = pUserData;
    return 0;
}


int LayerDequeueBuffer(LayerCtrl* l, VideoPicture** ppBuf, int bInitFlag)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    
    return LAYER_RESULT_USE_OUTSIDE_BUFFER;
}



int LayerQueueBuffer(LayerCtrl* l, VideoPicture* pBuf, int bValid)
{
    LayerCtrlContext* lc;
    int               i;
    VPictureNode*     newNode;
    VPictureNode*     nodePtr;
   
    lc = (LayerCtrlContext*)l;

    if(bValid == 0)
    {
        if(pBuf != NULL)
            lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf);
        return 0;
    }
    
	if(pBuf != NULL)
	{
		lc->callback(lc->pUserData, MESSAGE_ID_LAYER_NOTIFY_BUFFER, pBuf);
		lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf);   	
    }

#if BUF_MANAGE
    //* attach the new picture to list and return the old picture.
    newNode = NULL;
    for(i=0; i<32; i++)
    {
        if(lc->picNodes[i].bUsed == 0)
        {
            newNode = &lc->picNodes[i];
            newNode->pNext              = NULL;
            newNode->bUsed              = 1;
            newNode->pPicture           = pBuf;
            newNode->pSecondPictureOf3D = NULL;
            break;
        }
    }
    if(i == 32)
    {
        loge("not enough picture nodes, shouldn't run here.");
        abort();
    }

    if(lc->pPictureListHead != NULL)
    {
        nodePtr = lc->pPictureListHead;
        i = 1;
        while(nodePtr->pNext != NULL)
        {
            i++;
            nodePtr = nodePtr->pNext;
        }
        
        nodePtr->pNext = newNode;
        i++;
        
        //* return one picture.
        while(i > NUM_OF_PICTURES_KEEP_IN_LIST)
        {
            nodePtr = lc->pPictureListHead;
            lc->pPictureListHead = lc->pPictureListHead->pNext;
            lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)nodePtr->pPicture);
            nodePtr->pPicture = NULL;
            
            if(nodePtr->pSecondPictureOf3D != NULL)
            {
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)nodePtr->pSecondPictureOf3D);
                nodePtr->pSecondPictureOf3D = NULL;
            }
            nodePtr->bUsed = 0;
            i--;
        }
    }
    else
    {
        lc->pPictureListHead = newNode;
    }
#endif

    return 0;
}


int LayerDequeue3DBuffer(LayerCtrl* l, VideoPicture** ppBuf0, VideoPicture** ppBuf1)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    *ppBuf0 = NULL;
    *ppBuf1 = NULL;
    
    return 0;
}


int LayerQueue3DBuffer(LayerCtrl* l, VideoPicture* pBuf0, VideoPicture* pBuf1, int bValid)
{
    LayerCtrlContext* lc;
    int               i;


    return 0;
}


static int SetLayerParam(LayerCtrlContext* lc)
{
    disp_layer_info   layerInfo;
    unsigned long     args[4];
    
    return 0;
}


static void setHwcLayerPictureInfo(LayerCtrlContext* lc,
                                   disp_layer_info*  pLayerInfo,
                                   VideoPicture*     pPicture,
                                   VideoPicture*     pSecondPictureOf3D)
{
	return ;
}


int LayerCtrlShowVideo(LayerCtrl* l)
{
    return 0;
}


int LayerCtrlHideVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;
    unsigned long     args[4];


    return 0;
}


int LayerCtrlIsVideoShow(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    return 0;
}


int LayerCtrlHoldLastPicture(LayerCtrl* l, int bHold)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;

    
    return 0;
}

int LayerSetRenderToHardwareFlag(LayerCtrl* l,int bFlag)
{
    return 0;
}

int LayerSetDeinterlaceFlag(LayerCtrl* l,int bFlag)
{
    return 0;
}

int LayerSetFullScreenDisplay(LayerCtrl* l,int bFullScreenDisplay)
{
    LayerCtrlContext* lc;
    
    lc = (LayerCtrlContext*)l;
    
    logw("LayerSetFullScreenDisplay, bFullScreenDisplay = %d", bFullScreenDisplay);

	
	return 0;
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

int LayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{

	return 0;
}

int LayerGetRotationAngle(LayerCtrl* l)
{
	return 0;
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
    LayerGetDisplayFPS:              LayerGetDisplayFPS           ,  
};


