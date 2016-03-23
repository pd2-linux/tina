
#ifndef LAYER_CONTROL
#define LAYER_CONTROL

//#include "videoDecComponent.h"
#include "vdecoder.h"

typedef void* LayerCtrl;

const int MESSAGE_ID_LAYER_RETURN_BUFFER = 0x31;
const int MESSAGE_ID_LAYER_NOTIFY_BUFFER = 0x32;

const int LAYER_RESULT_USE_OUTSIDE_BUFFER = 0x2;

typedef struct LayerControlOpsS LayerControlOpsT;

typedef int (*LayerCtlCallback)(void* pUserData, int eMessageId, void* param);


enum EPICTURE3DMODE
{
    PICTURE_3D_MODE_NONE = 0,
    PICTURE_3D_MODE_TWO_SEPERATED_PICTURE,
    PICTURE_3D_MODE_SIDE_BY_SIDE,
    PICTURE_3D_MODE_TOP_TO_BOTTOM,
    PICTURE_3D_MODE_LINE_INTERLEAVE,
    PICTURE_3D_MODE_COLUME_INTERLEAVE
};

enum EDISPLAY3DMODE
{
    DISPLAY_3D_MODE_2D = 0,
    DISPLAY_3D_MODE_3D,
    DISPLAY_3D_MODE_HALF_PICTURE
};


struct LayerControlOpsS
{

	LayerCtrl* (*LayerInit)(void*, int);

	void (*LayerRelease)(LayerCtrl* , int );

	int (*LayerSetExpectPixelFormat)(LayerCtrl* , enum EPIXELFORMAT );

	enum EPIXELFORMAT (*LayerGetPixelFormat)(LayerCtrl* );

	int (*LayerSetPictureSize)(LayerCtrl* , int , int );

	int (*LayerSetDisplayRegion)(LayerCtrl* , int , int , int , int );

	int (*LayerSetBufferTimeStamp)(LayerCtrl* , int64_t );
	
	int (*LayerDequeueBuffer)(LayerCtrl* , VideoPicture** , int);
	
	int (*LayerQueueBuffer)(LayerCtrl* , VideoPicture* , int);

	int (*LayerCtrlHideVideo)(LayerCtrl*);

	int (*LayerCtrlShowVideo)(LayerCtrl* );

	int (*LayerCtrlIsVideoShow)(LayerCtrl* );

	int (*LayerCtrlHoldLastPicture)(LayerCtrl* , int );

	int (*LayerSetRenderToHardwareFlag)(LayerCtrl* ,int );

	int (*LayerSetDeinterlaceFlag)(LayerCtrl* ,int );

	//* for old display 
	int (*LayerSetPicture3DMode)(LayerCtrl* , enum EPICTURE3DMODE );

	enum EPICTURE3DMODE (*LayerGetPicture3DMode)(LayerCtrl* );

	int (*LayerSetDisplay3DMode)(LayerCtrl* , enum EDISPLAY3DMODE );

	enum EDISPLAY3DMODE (*LayerGetDisplay3DMode)(LayerCtrl* );

	int (*LayerDequeue3DBuffer)(LayerCtrl* , VideoPicture** , VideoPicture** );
	
	int (*LayerQueue3DBuffer)(LayerCtrl* , VideoPicture* , VideoPicture* , int);

	int (*LayerGetRotationAngle)(LayerCtrl* );

	int (*LayerSetCallback)(LayerCtrl* , LayerCtlCallback , void* );

	//* end 

	//* for new display
	int (*LayerSetBufferCount)(LayerCtrl* , int );

	int (*LayerSetVideoWithTwoStreamFlag)(LayerCtrl* , int );

	int (*LayerSetIsSoftDecoderFlag)(LayerCtrl* , int);

	void (*LayerResetNativeWindow)(LayerCtrl* ,void*);

	int (*LayerReleaseBuffer)(LayerCtrl* ,VideoPicture*);

	VideoPicture* (*LayerGetPicNode)(LayerCtrl* );

	int (*LayerGetAddedPicturesCount)(LayerCtrl* );

	int (*LayerGetDisplayFPS)(LayerCtrl* );
	
};

#endif

