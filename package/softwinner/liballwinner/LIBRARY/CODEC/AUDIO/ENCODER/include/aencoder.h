#ifndef AUDIO_ENC_API_H
#define AUDIO_ENC_API_H

#ifdef __cplusplus
extern "C" {
#endif



#include "aenc_sw_lib.h"

#define FIFO_LEVEL	(32)

typedef struct AudioEncoder AudioEncoder;
struct AudioEncoder
{
	
};

enum AUDIO_ENCODER_TYPE
{
       AUDIO_ENCODER_AAC_TYPE,
       AUDIO_ENCODER_LPCM_TYPE,    //only used by mpeg2ts
       AUDIO_ENCODER_PCM_TYPE,
       AUDIO_ENCODER_MP3_TYPE,
};

typedef enum AUDIO_ENCODER_TYPE AUDIO_ENCODER_TYPE;

typedef struct AudioEncConfig
{
	AUDIO_ENCODER_TYPE nType;
	int     nInSamplerate;   //输入fs
    int     nInChan;         //输入pcm chan 1:mon 2:stereo
    int     nBitrate;        //bs
    int     nSamplerBits;    //only for 16bits
    int     nOutSamplerate;  //输出fs,now OutSamplerate must equal InSamplerate
    int     nOutChan;        //编码输出 chan
    int     nFrameStyle;    //for aac: 0:add head,1:raw data; for pcm: 2:mpegTs pcm(big endian), other: common pcm(little endian)
}AudioEncConfig;


AudioEncoder* CreateAudioEncoder();
void DestroyAudioEncoder(AudioEncoder* pEncoder);
int InitializeAudioEncoder(AudioEncoder *pEncoder, AudioEncConfig *pConfig);
int ResetAudioEncoder(AudioEncoder* pEncoder);
int EncodeAudioStream(AudioEncoder *pEncoder);
int WriteAudioStreamBuffer(AudioEncoder *pEncoder, char* pBuf, int len);
int RequestAudioFrameBuffer(AudioEncoder *pEncoder, char **pOutBuf, unsigned int *size, long long *pts, int *bufId);
int ReturnAudioFrameBuffer(AudioEncoder *pEncoder, char *pOutBuf, unsigned int size, long long pts, int bufId);


#ifdef __cplusplus
}
#endif

#endif // AUDIO_ENC_API_H

