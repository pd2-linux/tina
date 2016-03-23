/*
*********************************************************************************************************
*                                                CedarX Framework
*                                   the Multimedia Framework for Linux/Android System
*
*                                      (c) Copyright 2010-2011, China
*                                              All Rights Reserved
*
* Version: V1.0
* By     : Softwinner Multimedia Group
*********************************************************************************************************
*/

#ifndef SUB_DEMX_H_
#define SUB_DEMX_H_

//#include <CDX_PlayerAPI.h>

//#include "subTotalInf.h"
#include "sdecoder.h"

typedef struct CdxSubDecoder
{
	void*         (*Init)(SubtitleStreamInfo* pSubStreamInfo);
    void          (*Exit)(void* pCodecContext);
    void          (*Reset)(void* pCodecContext, int64_t nSeekTime);
    int           (*Decode)(void* pCodecContext, SubtitleStreamDataInfo* pStreamData);
    SubtitleItem* (*NextSubItemInfo)(void* pCodecContext);
    SubtitleItem* (*ReqeustSubItem)(void* pCodecContext);
    void          (*FlushSubItem)(void* pCodecContext,SubtitleItem* pItem);
    
}CdxSubDecoder; 

extern CdxSubDecoder SubDecoderExternal;
extern CdxSubDecoder SubDecoderInternal;
extern CdxSubDecoder SubDecoderIdxsub;

#endif
/* File EOF */
