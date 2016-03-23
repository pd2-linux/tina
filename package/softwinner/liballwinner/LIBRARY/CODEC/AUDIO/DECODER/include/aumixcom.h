
#ifndef	___aumixcommon__
#define	___aumixcommon__
typedef	struct __PcmInfo__{
//input infor
	unsigned int		SampleRate;
	short*	PCMPtr;
	unsigned int		PcmLen;//sample num;
	unsigned int		Chan;
	int		preamp;//-20 -- 20 db	
}PcmInfo;

typedef	struct ___AudioMix__{
//input para
	PcmInfo	*InputA;
	PcmInfo	*InputB;
	PcmInfo	*TempBuf;
	PcmInfo	*Output;
	unsigned int	ByPassflag; //0: need mix A+B ==??∟A  1?那obypass   A==>A
//output para
	short*	MixbufPtr;
	unsigned int	MixLen;
	unsigned int    Mixch;
	void*           RESI;
}AudioMix;
int		do_AuMIX(AudioMix	*AMX);
void*   Init_ResampleInfo();
void    Destroy_ResampleInfo(void * ri);

//return?那ooutput sample num
//int		AudioResample(PcmInfo *Input,PcmInfo *Output);
//“～?InputA?a?“∩?芍??那?“o?3?“a?∫|足“∟“oy“??InputA?“∟“a???那?∟|足???|足?aoutput|足?sample“oy
int	AudioMixFuc(PcmInfo *InputA,PcmInfo *InputB,PcmInfo *Output) ;
#endif
