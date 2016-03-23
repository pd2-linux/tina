#ifndef CDX_AAC_MUXER_H
#define CDX_AAC_MUXER_H

#include "log.h"
#include "CdxMuxer.h"

typedef enum tag_FSWRITEMODE 
{
    FSWRITEMODE_CACHETHREAD = 0,
    FSWRITEMODE_SIMPLECACHE,
    FSWRITEMODE_DIRECT,
}FSWRITEMODE;

#endif /* CDX_AAC_MUXER_H */