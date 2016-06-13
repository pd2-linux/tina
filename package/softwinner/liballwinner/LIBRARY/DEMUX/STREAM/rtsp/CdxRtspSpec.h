#ifndef CEDARX_RTSP_SPEC_H
#define CEDARX_RTSP_SPEC_H

typedef struct CdxRtspPktHeaderS
{
    cdx_int32  type;
    cdx_uint32 length;
    cdx_int64  pts;
}CdxRtspPktHeaderT;

#endif

