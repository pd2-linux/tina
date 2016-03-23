#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "IEC61937.h"

int add61937Head(void *outint,void * tempshort, int samples)
{
	int ret =0;
	int i;
	int * out = (int*)outint;
	short* temp = (short*) tempshort;
	static int numtotal = 0;
    unsigned int channel_status[192];
	union head61937
    {
     headbpcuv head0;
     unsigned char head1;
    }head;
	word w1;
	samples>>=1;
	head.head0.other = 0;
	head.head0.B = 1;
	head.head0.P = 0;
	head.head0.C = 0;
	head.head0.U = 0;
	head.head0.V = 1;

	for (i=0 ; i<192; i++)
	{
		channel_status[i] = 0;
	}
	channel_status[1] = 1;
	//sample rates
	channel_status[24] = 0;
	channel_status[25] = 1;
	channel_status[26] = 0;
	channel_status[27] = 0;

	for (i = 0 ;i<samples;i++,numtotal++) {
		if( (numtotal%384 == 0) || (numtotal%384 == 1) )
		{
			head.head0.B = 1;
		}
		else
		{
			head.head0.B = 0;
		}
		head.head0.C = channel_status[(numtotal%384)/2];

		if(numtotal%384 == 0)
		{
			numtotal = 0;
		}

		w1.wval = (*temp)&(0xffff);

#if 1
		head.head0.P = w1.bits.bit15 ^ w1.bits.bit14 ^ w1.bits.bit13 ^ w1.bits.bit12
		              ^w1.bits.bit11 ^ w1.bits.bit10 ^ w1.bits.bit9 ^ w1.bits.bit8
		              ^w1.bits.bit7 ^ w1.bits.bit6 ^ w1.bits.bit5 ^ w1.bits.bit4
		              ^w1.bits.bit3 ^ w1.bits.bit2 ^ w1.bits.bit1 ^ w1.bits.bit0;
#endif

		ret = (int)(head.head1)<<24;
		ret |= (int)((w1.wval)&(0xffff))<<11;//8 or 12

		*out = ret;
		out++;
		temp++;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */


