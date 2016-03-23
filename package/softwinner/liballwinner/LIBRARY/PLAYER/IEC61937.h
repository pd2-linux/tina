#ifndef IEC61937_H
#define IEC61937_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IEC61937 API
 */

typedef struct headbpcuv{
	unsigned other:3;
    unsigned V:1;
    unsigned U:1;
    unsigned C:1;
    unsigned P:1;
    unsigned B:1;
} headbpcuv;

typedef union word
{
	struct
	{
		unsigned int bit0:1;
		unsigned int bit1:1;
		unsigned int bit2:1;
		unsigned int bit3:1;
		unsigned int bit4:1;
		unsigned int bit5:1;
		unsigned int bit6:1;
		unsigned int bit7:1;
		unsigned int bit8:1;
		unsigned int bit9:1;
		unsigned int bit10:1;
		unsigned int bit11:1;
		unsigned int bit12:1;
		unsigned int bit13:1;
		unsigned int bit14:1;
		unsigned int bit15:1;
		unsigned int rsvd:16;
	}bits;
	unsigned int wval;
}word;

int add61937Head(void *out,void * temp, int samples);

#ifdef __cplusplus
}
#endif


#endif


