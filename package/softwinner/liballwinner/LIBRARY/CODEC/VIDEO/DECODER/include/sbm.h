
#ifndef SBM_H
#define SBM_H

#include "vdecoder.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STREAMFRAMEFIFO
{
    VideoStreamDataInfo* pFrames;
    int                  nMaxFrameNum;
    int                  nValidFrameNum;
    int                  nUnReadFrameNum;
    int                  nReadPos;
    int                  nWritePos;
    int                  nFlushPos;
}StreamFrameFifo;


typedef struct STREAMBUFFERMANAGER
{
    pthread_mutex_t mutex;
    char*           pStreamBuffer;
    char*           pStreamBufferEnd;
    int             nStreamBufferSize;
    char*           pWriteAddr;
    int             nValidDataSize;
    StreamFrameFifo frameFifo;
}Sbm;


Sbm* SbmCreate(int nBufferSize);

Sbm *SbmSecureosCreate(int nBufferSize);

Sbm *SbmVirCreate(int nBufferSize);

void SbmDestroy(Sbm* pSbm);

void SbmSecureosDestroy(Sbm *pSbm);

void SbmVirDestroy(Sbm *pSbm);

void SbmReset(Sbm* pSbm);

void* SbmBufferAddress(Sbm* pSbm);

int SbmBufferSize(Sbm* pSbm);

int SbmStreamFrameNum(Sbm* pSbm);

int SbmStreamDataSize(Sbm* pSbm);

int SbmRequestBuffer(Sbm* pSbm, int nRequireSize, char** ppBuf, int* pBufSize);

int SbmAddStream(Sbm* pSbm, VideoStreamDataInfo* pDataInfo);

VideoStreamDataInfo* SbmRequestStream(Sbm* pSbm);

int SbmReturnStream(Sbm* pSbm, VideoStreamDataInfo* pDataInfo);

int SbmFlushStream(Sbm* pSbm, VideoStreamDataInfo* pDataInfo);
char* SbmBufferWritePointer(Sbm* pSbm);

void* SbmBufferDataInfo(Sbm* pSbm);

#ifdef __cplusplus
}
#endif
#endif

