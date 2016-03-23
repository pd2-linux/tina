#ifndef AUDIODEC_DECODE_H
#define AUDIODEC_DECODE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define cdx_mutex_lock(x)	pthread_mutex_lock(x)
#define cdx_mutex_unlock(x)  pthread_mutex_unlock(x)

typedef void AudioDecoderLib;

int ParseRequestAudioBitstreamBuffer(AudioDecoderLib* pDecoder,
               int              nRequireSize,
               unsigned char**  ppBuf,
               int*             pBufSize,
               unsigned char**  ppRingBuf,
               int*             pRingBufSize,
               int*             nOffset);
int ParseUpdateAudioBitstreamData(AudioDecoderLib* pDecoder,
               int     nFilledLen,
               int64_t nTimeStamp,
               int     nOffset);
int ParseAudioStreamDataSize(AudioDecoderLib* pDecoder);
void BitstreamQueryQuality(AudioDecoderLib* pDecoder, int* pValidPercent, int* vbv);
void ParseBitstreamSeekSync(AudioDecoderLib* pDecoder, int64_t nSeekTime, int nGetAudioInfoFlag);

int InitializeAudioDecodeLib(AudioDecoderLib*    pDecoder,
               AudioStreamInfo* pAudioStreamInfo,
               BsInFor *pBsInFor);
int DecodeAudioFrame(AudioDecoderLib* pDecoder,
               char*        ppBuf,
               int*          pBufSize);
int DestroyAudioDecodeLib(AudioDecoderLib* pDecoder);

void SetAudiolibRawParam(AudioDecoderLib* pDecoder, int commond);

AudioDecoderLib* CreateAudioDecodeLib(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif//AUDIODEC_DECODE_H
