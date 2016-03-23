#ifndef SUBREADER_H
#define SUBREADER_H

//#include <CDX_MemWatch.h>
//#include <cedarx_demux.h>
#include "subDmx.h"
#include "subTotalInf.h"
#include "transfromType.h"
#include <pthread.h>
//#include <CDX_Subtitle.h>
//#include <cedarx_stream_file.h>

#define ERR ((void *) -1)
#define LINE_LEN 1024
#define MAX_SUBTITLE_FILES 32
#define CEDARV_FLAG_PTS_VALID   0x2
#define CEDARV_FLAG_FIRST_PART  0x8
#define CEDARV_FLAG_LAST_PART   0x10

#define SUB_MAX_ITEM_NUM     (6)
#define SUB_MAX_BUF_SIZE     (128*1024) //(128*1024)(2*1024*1024)
#define SUB_MAX_IDX_NUM      (64)
#define SUB_MAX_ANCIBUF_SIZE (64*1024)
#define SUB_MAX_COPY_BUFFER_SIZE (64*1024)

#define SUB_TEXT_SIZE       (256)
#define SUB_DVDSUB_SIZE     (1024*1024)
#define SUB_PGS_SIZE        (1*1024*1024)
#define SUB_DIVX_SIZE        (1*1024*1024)

#define SUB_MIDDLE_BUF_SIZE (512*1024)
#define PGS_MAX_RLE_SIZE    (128*1024)
#define PGS_FRAME_SIZE		(300*1024)

#define SUB_DEC_STATUS_FALG      1
#define SUB_RELEASE_STATUS_FALG  0
#define SUB_DISP_STATUS_FALG     3

#define SUB_ANCI_STREAM_BUFFER_SIZE (16*1024)

#define TV_SYSTEM_NTSC           0
#define TV_SYSTEM_PAL            1
#define TOP_FIELD_FLAG           0
#define BOTTOM_FIELD_FLAG        1

#define MAX_NEG_CROP	1024
#define USE_16_COLOR_PALLETE 0

#define SUB_DEC_NO_ENOUGH_DATA    3
#define SUB_DEC_END_DISP          5

#define SUB_DIVX_DXSA 1
#define SUB_DIVX_DXSB 0

#define TEXT_SUBTITLE_DATA_SIZE (8*1024)

enum
{
    SUB_RENDER_ALIGN_NONE       = 0,
    SUB_RENDER_HALIGN_LEFT      = 1,
    SUB_RENDER_HALIGN_CENTER    = 2,
    SUB_RENDER_HALIGN_RIGHT     = 3,
    SUN_RENDER_HALIGN_MASK      = 0x0000000f,
    SUB_RENDER_VALIGN_TOP       = (1 << 4),
    SUB_RENDER_VALIGN_CENTER    = (2 << 4),
    SUB_RENDER_VALIGN_BOTTOM    = (3 << 4),
    SUN_RENDER_VALIGN_MASK      = 0x000000f0
};

typedef struct CedarXExternFdDesc{
	int		fd;   //SetDataSource FD
	long long offset;
	//recoder where fd is now.
	long long cur_offset;
	long long length;
}CedarXExternFdDesc;

enum
{
   SUB_STYLE_NONE          = 0,
   SUB_STYLE_BOLD          = 1<<0,
   SUB_STYLE_ITALIC        = 1<<1,
   SUB_STYLE_UNDERLINE     = 1<<2,
   SUB_STYLE_STRIKETHROUGH = 1<<3
};

enum  EXTERNAL_SUBTITLE_FORMAT
{
	// subtitle formats
	SUB_INVALID   =    -1,
	SUB_SAMI      =     0,
	SUB_SSA       =     1,
	SUB_IAUTHORSCRIPT  = 2,
	SUB_CPC600         = 3,
	SUB_TIME_SAMELINE  = 4,
	SUB_TIME_DIFFLINE  = 5,
	SUB_TIME_USEFRAME  = 6,
	SUB_ASC            = 7,
	SUB_DAT            = 8,
	SUB_TMPLAYER       = 9,
	SUB_RTF            = 10,
	SUB_S2K            = 11,
	SUB_SONIC          = 12,
	SUB_INSCRIBERCG    = 13,
	SUB_MPSUB          = 14,
	SUB_TIME_AFTER     = 15,
	SUB_SOFNI          = 16,
	SUB_IDXSUB        = 17,
	SUB_UNKNOWN       = 18
};

typedef struct SUB_DATA_INF
{
    SubtitleItem* subItemInf;
    OMX_U32       subItemNums;
}sub_data_inf;



typedef struct SUBTITLE_EXTERNAL_INF
{
    OMX_S8            dataFormat;           // SUB_DATA_STRUCT_ARGB, SUB_DATA_STRUCT_RGBA, SUB_DATA_STRUCT_BGRA, SUB_DATA_STRUCT_ABGR,
    OMX_S32           dataEncodingType;
    OMX_S32           subFormat;
    OMX_U32           frameRate;
    OMX_U32           subLangNums; //**this variable should motify
    sub_data_inf*     subDataInf[MAX_SUBTITLE_FILES];
    OMX_S8            subFormatCheckString[256];
    OMX_S8            subFormatCheckStringIdx;
    OMX_U32           perFrameDuration;// us
    OMX_U8            recordLastLine[LINE_LEN+1];
    OMX_S32           preTime;
    OMX_U32           hasDecSubItemNum[32];
    void*             subDecAnciInf;
    OMX_S32           dispSubIndex;
    OMX_U32           updateSubItemIdx;

    //OMX_U32           screenWidth;
    //OMX_U32           screenHeight;
    OMX_U8            subLangIdx[32][32];
    SubtitleItem*     dispSubItem;
    OMX_U32           minDispEndTime;
    OMX_U8            needSearchSuitSubFlag;
    OMX_U8            bSmiHaveHeadInfoFlag;


    CedarXExternFdDesc subFdDesc;
    CedarXExternFdDesc idxFdDesc;
}sub_external_inf;

typedef struct DVD_SUB_INF
{
    OMX_U8   contrast[4];
    OMX_U8   colorCode[4];
    OMX_U8   tvType;
    OMX_U32  orgSubWidth;
    OMX_U32  orgSubHeight;
}dvd_sub_inf_t;


typedef struct PGS_SUB_PRESENTATION
{
    OMX_S32 nStartX;
    OMX_S32 nStartY;
    OMX_U32 videoWidth;
    OMX_U32 videoHeight;
    OMX_S32 idNumber;
}pgs_sub_present_t;

typedef struct PGS_SUB_PICTURE
{
    OMX_S32 width;
    OMX_S32	height;
    OMX_U32	rleBufSize;
	OMX_U32	rleDataLen;
    OMX_U8	rle[PGS_MAX_RLE_SIZE];
} pgs_sub_picture_t;

typedef enum PGS_DECODE_STATE
{
    NEED_PRESENTATION_SEGMENT,
    NEED_WINDOW_SEGMENT,
    NEED_PALLETE_SEGMENT,
    NEED_PICTURE_SEGMENT,
    NEED_DISPLAY_SEGMENT
}pgs_dec_state;


typedef struct PGS_SUB_INF
{
    pgs_dec_state       state;
    OMX_U32             blockId;
    OMX_S64             lastDecodedPts;
    OMX_S64             newSubPts;
    pgs_sub_present_t	presentation;
    pgs_sub_picture_t   picture;
#if !USE_16_COLOR_PALLETE
    OMX_U32				clut[256];
	OMX_U8				cropTbl[256 + 2*MAX_NEG_CROP];
#else
    OMX_U8				clut[256];
#endif
}pgs_sub_inf_t;



typedef struct SUB_DISPBUF_MANAGER
{
    SubtitleItem subItemInf[SUB_MAX_ITEM_NUM];
    OMX_U8       nWriteSubIdx;
    OMX_U8       nReadSubIdx;
    OMX_U8       validSubNum;
    OMX_U8       nEmptySubNum;
    OMX_U32      nSubItemId;
    pthread_mutex_t subFrameMutex;
}sub_dispbuf_manager_t;

typedef struct SUB_BUF_COONTROL_INF
{
    OMX_U32 ptsVal;
    OMX_U32 duration;
    OMX_U8  ptsValid;
    OMX_U8* startAddr;
    OMX_U32 validSize;
}sub_buf_ctrl_t;

typedef struct SUB_INTERNAL_INF
{
    OMX_S8           dataFormat;
    OMX_U8           aIndex;
    OMX_U8           rIndex;
    OMX_U8           gIndex;
    OMX_U8           bIndex;
    OMX_S32          dataEncodingType;
    OMX_U32          frameWidth;
    OMX_U32          frameHeight;
    OMX_U32          frameRate;
    ESubtitleCodec   subType;
    OMX_S64          ptsVal;
    OMX_S64          uDuration;
    OMX_U32          endPtsVal;
    void*            subAnciDecInf;
    OMX_U8*          midSubDataBuf;
    sub_dispbuf_manager_t*  subDispManager;
    SubtitleItem*           dispSubItem;
    OMX_U8                  paletteValidFlag;
    OMX_S32                 paletteAry[32];
    OMX_U32                 recordSubNum;
    SubtitleItem*           lastDecsubItem;
    SubtitleStreamDataInfo* pStreamData;
    SubtitleStreamDataInfo  mAnciStreamDataT;
    OMX_U8 bSubDivxABFlag;
    OMX_U8 bTextSubtitleFlag;
    SubtitleStreamDataInfo  mTextSubtitleStreamData;
   // OMX_U32   screenWidth;
   // OMX_U32   screenHeight;
    OMX_S8 (*sub_dec)(struct SUB_INTERNAL_INF *subInternalInf);

}sub_internal_inf;

typedef struct SSA_ASS_CONTROL_INF
{
    OMX_U8 styleName[64];
    OMX_U8 fontName[64];
    OMX_S8 fontNameIdx;
    OMX_U8 nFontSize;
    OMX_S32 mainAligment;
    OMX_U32 marginL;
    OMX_U32 marginR;
    OMX_U32 marginV;
    OMX_U32 encoding;
    OMX_U32 nPrimaryColor;
    OMX_U32 nSecondaryColor;
    OMX_U32 outlineColour;
    OMX_U32 backColour;
    OMX_U32 subStyle;
    OMX_U32 scalerx;
    OMX_U32 scalery;
    OMX_U32 rotateAngle;
}ssa_ass_control_inf;

typedef struct SSA_ASS_SUB_INF
{
    OMX_U8  isAssSubFlag;
    OMX_U8  styleNum;
    OMX_U32 playResX;
    OMX_U32 playResY;
   // OMX_U32 screenWidth;
  //  OMX_U32 screenHeight;
    ssa_ass_control_inf* ssaAssCtlInf;
}ssa_ass_sub_inf_t;


typedef struct TIMED_TEXT_SUB_INF
{
   OMX_U32 displayFlags; //0x20: scroll int; 0x40: scroll out; 0x180:scroll direction; 0x800: continuous karaoke;
                         //0x20000:write text vertically; 0x40000: fill text region
   OMX_S32 eAlignment;
   OMX_U32 backGroundColor;
   OMX_U16 boxTop;
   OMX_U16 boxLeft;
   OMX_U16 boxBottom;
   OMX_U16 boxRight;
   OMX_U8  fontName[64];
   OMX_U32 subStyle;
   OMX_U8  nFontSize;
   OMX_U32 textColor;
   OMX_U8  ctrlInfValid;
}ttxt_sub_inf_t;



typedef struct subreader {
    SubtitleItem* (*read)(sub_external_inf* subExternalInf, int fd, SubtitleItem **current, int utf16);
    void       (*post)(SubtitleItem *dest);
    const OMX_STRING name;
}sub_reader_t;


//*******************************************************************************************************************//
//*******************************************************************************************************************//
typedef struct IDXSUB_ITEM_INF       //the information of each subItem
{
    int64_t  nPts;
    int64_t  endTime;
    OMX_U32  fileStartPos;       // the start pos of the sub in the *.sub file
    OMX_U32  fileEndPos;         // the end pos of the sub in the *.sub file
}idxSubtitleItem;

typedef struct IDXSUB_LANG_INF
{
    idxSubtitleItem* subItemInf;   // the informatiob of each sub item
    OMX_U32          subItemNums;  // the sub nums of cur sub language
    OMX_U8           subLangIdx;   // the sub index 0~32
    OMX_U8           subLangName[32];
}idxsub_lang_inf;


#define IDXSUB_MAX_LANG_NUM 32

typedef struct IDXSUB_INF
{
	void*             priv_data;
    OMX_U8            curSubLangIdx;
    OMX_U8            paletteFlag;
    OMX_U32           subFrameWidth;
    OMX_U32           subFrameHeight;
    int64_t           subTimeLen;
    OMX_U32           subLangNums;
    OMX_S32           paletteArray[16];
    OMX_U32           curUpdateSubIdx;
    OMX_U32           hasReadSubPos;
    OMX_U32           hasUpdateSubIdx;
    idxsub_lang_inf*  curSubLangInf[IDXSUB_MAX_LANG_NUM];
    CedarXExternFdDesc subFdDesc;
    CedarXExternFdDesc idxFdSec;
    SubtitleStreamDataInfo mStreamDataT;
    OMX_S32           nStreamDataMaxSize;
    OMX_S32           nSubLangNum;
}idxsub_inf;

typedef struct DivxSubInfS
{
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_U32 nTop;
    OMX_U32 nBottom;
    OMX_U32 nLeft;
    OMX_U32 nRight;
    OMX_U32 nFieldOffset;
    OMX_U32 pallteArry[4][4];
}DivxSubInfT;
#endif /* MPLAYER_SUBREADER_H */
