#ifndef	__BsInfoType__
#define	__BsInfoType__
#include "adecoder.h"

typedef struct __AFRAME_INFO
{
    unsigned char    *startAddr;
    unsigned int    ptsValid;//0:��ǰʱ�����Ч��1����ǰʱ�����Ч    
    int64_t         pts;
    unsigned int    len;
    unsigned int    readlen;	
}aframe_info_t;

typedef struct __ASTREAM_FIFO
{
    aframe_info_t 	*inFrames;
    unsigned int    rdIdx;//
    unsigned int    wtIdx;//
    int             ValidchunkCnt;//��Ч��packet��Ŀ//0:��ǰ֡û����ݣ�1:��ǰ֡�����
    unsigned int    maxchunkNum;
    int64_t         NowPTSTime;
    int64_t         dLastValidPTS;
        
}astream_fifo_t;

typedef struct __Ac320FileRead
{
    unsigned int    FileLength;             //�ļ��ܳ���
    unsigned int    FileReadingOpsition;    //�ļ��´�Ҫ����λ�õ�ƫ��
    unsigned int    FileWritingOpsition;    //buffer�´�Ҫд��λ�õ�ƫ��
    unsigned int    need_offset;
    unsigned short  updataoffsetflag;
    unsigned short  BigENDFlag;             //1 big 0 little
    unsigned char   *bufStart;              //buffer�׵�ַ
    int             BufToTalLen;            //�ܳ���
	
    unsigned char   *bufReadingPtr;         //���ڶ���ָ��
    int             BufLen;                 //��Ч��ݳ���
	
    unsigned char   *bufWritingPtr;         //���ڶ���ָ��
    int             BufValideLen;           //���೤��
    //unsigned int    ReadingOffset;
    astream_fifo_t 	frmFifo;
    //add for android
    void*           tmpGlobalAudioDecData;
	
}Ac320FileRead;

#define	AUD_TAG_INF_SIZE 512
typedef  struct com_internal_struct
{
    //0x88
    unsigned int             *pdecInfoSet;
    unsigned int             *pBackupBufferPoint;
    unsigned int             ulBitstreamAddPtr;
    unsigned int             ulDspVersionNum;
    unsigned int             ulPlayStartTime;
    unsigned int             ulPlayEndTime;
    unsigned int             ulBreakSentenceEnergy;
    unsigned int             ulBreakSentenceTime;
    unsigned int             ulFileSumData;
    unsigned int             ulForwardBackSilenceTime;
    unsigned int             ulForwardBackPlayTime;
    unsigned int             *SptrumoutAdd;
    unsigned int             *UserEqinputAdd;

    unsigned short           ulBackupLength;
                             
    unsigned char            ulNormalPlay;               //=ulFadeInOutFinish
    unsigned char            ulDecComm;
    unsigned char            ulFadeinFlag;
    //0x8b                   
    unsigned char            ulFadeInOutFinish;
    //0x89
    unsigned int             ulTotalTimeMS;

    unsigned int             ulAverageBit;
    int /*unsigned short*/   ulSampleRate;               //?why not this data is fact data;
    //0x8a
    unsigned char            ulExitMean;
    unsigned int             ulDebugInfo;
    //0x60                   
    unsigned int             ulNowTimeMS;
    unsigned int             ulNowTimeMSAPoint;
    unsigned int             ulBitrate;
    unsigned char            ulMode;
    unsigned char            ulFFREVDoing;
    //0x70
    signed char              ulVpsSet;
    unsigned int/*char*/     ulFFREVStep;
    unsigned char            ulFFREVFlag;
    unsigned char            ulABSet;
    unsigned char            ulEQFlag;
    unsigned short           ulUserEq[10];
    unsigned char            ulspectrum;
    unsigned short           Hmspetrum;//1-8
    unsigned short           ulspectrumval[8][32];
    unsigned char            ulChannelControl;
    unsigned char            ulVolumeSet;
    //0x61 a sentence end
    //0x62 dsp mips limit
    //0x63 dsp alive minimum time
    //0x40 decoder need new bitstream to decode
    unsigned int             ulReadBitstreamLen;
    int                      ulSkipPageLength;
    unsigned char            urFadeOutFlag;
    unsigned char            urFileFlag;
    unsigned int             urTrueReadLen;
    unsigned int             ulFilecurrentpage;
    //0x41
    unsigned char            ulIndex;
    unsigned int             ulDestinationAdd;
    //0x42
    unsigned char            ulIndex1;
    unsigned int             ulDestinationAdd1;
    //0x43
    unsigned int             bWithVideo;
    unsigned int             ulDestinationAdd2;
    //0x21
    unsigned int             *ulBufferAddress;
    unsigned int             ulBufferLength;

    unsigned short           NeedSetPlayTimeFlag;
    unsigned int             framecount;

    unsigned char            AudTagInfo[AUD_TAG_INF_SIZE];

} com_internal;

typedef struct __AudioDEC_AC320
{
    Ac320FileRead *FileReadInfo;
    BsInFor       *BsInformation;
    com_internal  *DecoderCommand;
    int           Decinitedflag;
    int           (*DecInit)(struct __AudioDEC_AC320 *p);
    int           (*DecFrame)(struct __AudioDEC_AC320 *p,char *OutBuffer,int *OutBuffLen);
    int           (*DecExit)(struct __AudioDEC_AC320 *p);
 
}AudioDEC_AC320;

struct __AudioDEC_AC320 *AudioAACDecInit(void);
int	 AudioAACDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioAXXDecInit(void);
int	 AudioAXXDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioAPEDecInit(void);
int	 AudioAPEDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioDXXDecInit(void);
int	 AudioDXXDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioCOOKDecInit(void);
int	 AudioCOOKDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioSIPRDecInit(void);
int	 AudioSIPRDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioATRCDecInit(void);
int	 AudioATRCDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioOGGDecInit(void);
int	 AudioOGGDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioWAVDecInit(void);
int	 AudioWAVDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioAMRDecInit(void);
int	 AudioAMRDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioFLACDecInit(void);
int	 AudioFLACDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioWMADecInit(void);
int	 AudioWMADecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioMP3DecInit(void);
int	 AudioMP3DecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioRADecInit(void);
int	 AudioRADecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioALACDecInit(void);
int	 AudioALACDecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioG729DecInit(void);
int	 AudioG729DecExit(struct __AudioDEC_AC320 *p);
struct __AudioDEC_AC320 *AudioDSDDecInit(void);
int	 AudioDSDDecExit(struct __AudioDEC_AC320 *p);





#endif
