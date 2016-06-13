/*
********************************************************************************
*                                    eMOD
*                   the Easy Portable/Player Develop Kits
*                               mod_herb sub-system
*                          (module name, e.g.mpeg4 decoder plug-in) module
*
*          (c) Copyright 2009-2010, Allwinner Microelectronic Co., Ltd.
*                              All Rights Reserved
*
* File   : avi_depack_core_cfg.h
* Version: V1.0
* By     : Eric_wang
* Date   : 2010-9-6
* Description:
********************************************************************************
*/
#ifndef _CDX_AVI_DEPACK_CORE_CFG_H_
#define _CDX_AVI_DEPACK_CORE_CFG_H_

#include <CdxTypes.h>

#define MAX_CHUNK_BUF_SIZE (1024*512)

//for keyframe table, 128k default 366600byte, index 366600/40byte = 9165 keyframe

#define MAX_IDX_BUF_SIZE                (1024 * 16 * 27)
#define MAX_IDX_BUF_SIZE_FOR_INDEX_MODE (1024 * 16 * 41)
#define INDEX_CHUNK_BUFFER_SIZE         (1024 * 128)    //for store index chunk in readmode_index
#define MAX_FRAME_CHUNK_SIZE            (2 * 1024 * 1024)   //max frame should not larger than 2M byte
#define AVI_STREAM_NAME_SIZE            (32)

#define MAX_READ_CHUNK_ERROR_NUM   (100)

//debug
#define PLAY_AUDIO_BITSTREAM    (1)
#define PLAY_VIDEO_BITSTREAM    (1)

#endif  /* _CDX_AVI_DEPACK_CORE_CFG_H_ */

