/*
 * tiny_mp3.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "libaudio/audio.h"
#include "mp3player.h"

//#include "utils_interface.h"

/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */

#if 0
#define DEBUG(x,y...)	(printf("DEBUG [ %s : %s : %d] "x"\n",__FILE__, __func__, __LINE__, ##y))
#else
#define DEBUG(x,y...)
#endif
#define ERROR(x,y...)	(printf("ERROR [ %s : %s : %d] "x"\n", __FILE__, __func__, __LINE__, ##y))

#define HDR_SIZE 4
//! how many valid frames in a row we need before accepting as valid MP3
#define MIN_MP3_HDRS 3//12
//! Used to describe a potential (chain of) MP3 headers we found
typedef struct mp3_hdr {
  off_t frame_pos; // start of first frame in this "chain" of headers
  off_t next_frame_pos; // here we expect the next header with same parameters
  int mp3_chans;
  int mp3_freq;
  int mpa_spf;
  int mpa_layer;
  int mpa_br;
  int cons_hdrs; // if this reaches MIN_MP3_HDRS we accept as MP3 file
  struct mp3_hdr *next;
} mp3_hdr_t;

static tiny_mp3_t *tiny = NULL;
static uint8_t Output[6912];
static int quit_flag = 0;

enum decoder_return {
	DECODER_OK		= 0,
	DECODER_STOP		= 1,
	DECODER_EOF		= 2,
	DECODER_UNRECOVERY	= -1,
};

static int decode_init(mad_decoder_t *decode)
{
	mad_synth_init(&decode->synth);
	mad_stream_init(&decode->stream);
	mad_frame_init(&decode->frame);

	decode->buf_size = 1152;
	decode->dbuf = malloc(decode->buf_size);
	if (!decode->dbuf) {
		ERROR("Alloc decode buffer: %s", strerror(errno));
		return -1;
	}

	return 0;
}

static void decode_deinit(mad_decoder_t *decode)
{
	mad_synth_finish(&decode->synth);
	mad_frame_finish(&decode->frame);
	mad_stream_finish(&decode->stream);

	if (decode->dbuf) {
		free(decode->dbuf);
		decode->dbuf = NULL;
	}
}

static void decode_zero_in(mad_decoder_t *decode, int fd)
{
	decode->in_buf_len	= 0;
	decode->eof		= 0;
	lseek(fd, 0, SEEK_SET);
}

static int decode_frame(mad_decoder_t *decode, int fd, int *quit)
{
	char *buf = decode->dbuf;
	ssize_t rSize;
	int ret;

	while (!(*quit)) {
		rSize = read(fd,
			     buf + decode->in_buf_len,
			     decode->buf_size - decode->in_buf_len);
		if (rSize == 0) {
			if (!decode->eof)
				decode->eof = 1;
		} else if (rSize < 0) {
			ERROR("read file: %s", strerror(errno));
			return DECODER_UNRECOVERY;
		}

		decode->in_buf_len += rSize;

		mad_stream_buffer(&decode->stream, (uint8_t *)buf, decode->in_buf_len);
		ret = mad_frame_decode(&decode->frame, &decode->stream);

		if (decode->stream.next_frame) {
			int num_bytes =
				(buf + decode->in_buf_len) - (char *)decode->stream.next_frame;
			memmove(buf, decode->stream.next_frame, num_bytes);
			decode->in_buf_len = num_bytes;
		}

		if (ret == 0)
			return DECODER_OK;

		if (ret < 0) {
			if (decode->eof) {
				return DECODER_EOF;
			} else if (MAD_RECOVERABLE(decode->stream.error) ||
			    decode->stream.error == MAD_ERROR_BUFLEN) {
				continue;
			} else {
				ERROR("mad decode:%s", mad_stream_errorstr(&decode->stream));
				return DECODER_UNRECOVERY;
			}
		}
	}

	return DECODER_STOP;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline
signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static void play_frame(mad_decoder_t *decode, audio_output_t *ao)
{
	mad_fixed_t const *left_ch, *right_ch;
	struct mad_pcm *pcm = &decode->synth.pcm;
	uint32_t nchannels, nsamples, n;
	uint8_t *OutputPtr;

	/* pcm->samplerate contains the sampling frequency */
	mad_synth_frame(&decode->synth, &decode->frame);

	nchannels	= pcm->channels;
	nsamples	= pcm->length;
	left_ch		= pcm->samples[0];
	right_ch	= pcm->samples[1];

	n = nsamples;
	OutputPtr = Output;

	while (nsamples--) {
		signed int sample;
		/* output sample(s) in 16-bit signed little-endian PCM */
		sample = scale(*left_ch++);
		*(OutputPtr++) = sample >> 0;
		*(OutputPtr++) = sample >> 8;
		if (nchannels == 2) {
			sample = scale(*right_ch++);
			*(OutputPtr++) = sample >> 0;
			*(OutputPtr++) = sample >> 8;
		}
	}

	if ((int)(OutputPtr - Output) > 1152 *4) {
		DEBUG("Output buffer over 1152 * 4");
	}

	ao->play((short *)Output, n);
}

static int play_device_try(char *interface)
{
	audio_output_t *ao;

	ao = audio_get_output(interface);
	if (!ao) {
		ERROR("NOT found audio interface");
		return -1;
	}

	if (ao->dev_try)
		return ao->dev_try() < 0 ? -1 : 0;

	return 0;
}

static void get_file_type(struct stat *st, char *type, int len)
{
	switch (st->st_mode & S_IFMT) {
	case S_IFBLK:
		strncpy(type, "block device", len);
		break;
	case S_IFCHR:
		strncpy(type, "character device", len);
		break;
	case S_IFDIR:
		strncpy(type, "directory", len);
		break;
	case S_IFIFO:
		strncpy(type, "FIFO/pipe", len);
		break;
	case S_IFLNK:
		strncpy(type, "symlink", len);
		break;
	default:
		strncpy(type, "unknown file", len);
	}
}

unsigned int id3v2_tag_size(uint8_t maj_ver, tiny_mp3_t *tiny) {
	unsigned int header_footer_size;
	unsigned int size;
	uint8_t data;
	int i;

	if(read(tiny->fd, &data, 1) <= 0)
		return -1;
	if(data == 0xff)
		return 0;
	if(read(tiny->fd, &data, 1) <= 0)
		return -1;
	header_footer_size = ((data & 0x10) && maj_ver >= 4) ? 20 : 10;

	size = 0;
	for(i = 0; i < 4; i++) {
		if(read(tiny->fd, &data, 1) <= 0)
			return -1;
		if (data & 0x80)
			return 0;
		size = size << 7 | data;
	}

	return header_footer_size + size;
}
//----------------------- mp3 audio frame header parser -----------------------

static const uint16_t tabsel_123[2][3][16] = {
   { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0},
     {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,0},
     {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,0} },

   { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,0},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0} }
};

static const int freqs[9] = { 44100, 48000, 32000,   // MPEG 1.0
                              22050, 24000, 16000,   // MPEG 2.0
                              11025, 12000,  8000};  // MPEG 2.5

/*
 * return frame size or -1 (bad frame)
 */
int mp_get_mp3_header(unsigned char* hbuf,int* chans, int* srate, int* spf, int* mpa_layer, int* br){
    int stereo,lsf,framesize,padding,bitrate_index,sampling_frequency, divisor;
    int bitrate;
    int layer;
    static const int mult[3] = { 12000, 144000, 144000 };
    uint32_t newhead =
      hbuf[0] << 24 |
      hbuf[1] << 16 |
      hbuf[2] <<  8 |
      hbuf[3];

    // head_check:
    if( (newhead & 0xffe00000) != 0xffe00000 ){
      DEBUG("head_check failed\n");
      return -1;
    }

    layer = 4-((newhead>>17)&3);
    if(layer==4){
      DEBUG("not layer-1/2/3\n");
      return -1;
    }

    sampling_frequency = (newhead>>10)&0x3;  // valid: 0..2
    if(sampling_frequency==3){
      DEBUG("invalid sampling_frequency\n");
      return -1;
    }

    if( newhead & (1<<20) ) {
      // MPEG 1.0 (lsf==0) or MPEG 2.0 (lsf==1)
      lsf = !(newhead & (1<<19));
      sampling_frequency += lsf*3;
    } else {
      // MPEG 2.5
      lsf = 1;
      sampling_frequency += 6;
    }

    bitrate_index = (newhead>>12)&0xf;  // valid: 1..14
    padding   = (newhead>>9)&0x1;

    stereo    = ( ((newhead>>6)&0x3) == 3) ? 1 : 2;

    bitrate = tabsel_123[lsf][layer-1][bitrate_index];
    framesize = bitrate * mult[layer-1];

    if(!framesize){
      DEBUG("invalid framesize/bitrate_index\n");
      return -1;
    }

    divisor = layer == 3 ? (freqs[sampling_frequency] << lsf) : freqs[sampling_frequency];
    framesize /= divisor;
    framesize += padding;
    if(layer==1)
      framesize *= 4;

    if(srate)
      *srate = freqs[sampling_frequency];
    if(spf) {
      if(layer == 1)
        *spf = 384;
      else if(layer == 2)
        *spf = 1152;
      else if(sampling_frequency > 2) // not 1.0
        *spf = 576;
      else
        *spf = 1152;
    }
    if(mpa_layer) *mpa_layer = layer;
    if(chans) *chans = stereo;
    if(br) *br = bitrate;

    return framesize;
}

static mp3_hdr_t *add_mp3_hdr(mp3_hdr_t **list, off_t st_pos,
                               int mp3_chans, int mp3_freq, int mpa_spf,
                               int mpa_layer, int mpa_br, int mp3_flen) {
  mp3_hdr_t *tmp;
  int in_list = 0;
  while (*list && (*list)->next_frame_pos <= st_pos) {
    if (((*list)->next_frame_pos < st_pos) || ((*list)->mp3_chans != mp3_chans)
         || ((*list)->mp3_freq != mp3_freq) || ((*list)->mpa_layer != mpa_layer) ) {
      // wasn't valid!
      tmp = (*list)->next;
      free(*list);
      *list = tmp;
    } else {
      (*list)->cons_hdrs++;
      (*list)->next_frame_pos = st_pos + mp3_flen;
      (*list)->mpa_spf = mpa_spf;
      (*list)->mpa_br = mpa_br;
      if ((*list)->cons_hdrs >= MIN_MP3_HDRS) {
        // copy the valid entry, so that the list can be easily freed
        tmp = malloc(sizeof(mp3_hdr_t));
        memcpy(tmp, *list, sizeof(mp3_hdr_t));
        tmp->next = NULL;
        return tmp;
      }
      in_list = 1;
      list = &((*list)->next);
    }
  }
  if (!in_list) { // does not belong into an existing chain, insert
    // find right position to insert to keep sorting
    while (*list && (*list)->next_frame_pos <= st_pos + mp3_flen)
      list = &((*list)->next);
    tmp = malloc(sizeof(mp3_hdr_t));
    tmp->frame_pos = st_pos;
    tmp->next_frame_pos = st_pos + mp3_flen;
    tmp->mp3_chans = mp3_chans;
    tmp->mp3_freq = mp3_freq;
    tmp->mpa_spf = mpa_spf;
    tmp->mpa_layer = mpa_layer;
    tmp->mpa_br = mpa_br;
    tmp->cons_hdrs = 1;
    tmp->next = *list;
    *list = tmp;
  }
  return NULL;
}

/**
 * \brief free a list of MP3 header descriptions
 * \param list pointer to the head-of-list pointer
 */
static void free_mp3_hdrs(mp3_hdr_t **list) {
  mp3_hdr_t *tmp;
  while (*list) {
    tmp = (*list)->next;
    free(*list);
    *list = tmp;
  }
}

static int parse_header(tiny_mp3_t *tiny, int* quit){
    int mp3_freq, mp3_chans, mp3_flen, mpa_layer, mpa_spf, mpa_br;
	mp3_hdr_t *mp3_hdrs = NULL, *mp3_found = NULL;
	uint8_t hdr[HDR_SIZE];
	off_t st_pos = 0;
	int step, rSize, n = 0;
	int ret = DECODER_STOP;

	rSize = read(tiny->fd, hdr, HDR_SIZE);
	while(n < 30000 && !(*quit)){
		st_pos = lseek(tiny->fd, 0, SEEK_CUR) - HDR_SIZE;
		step = 1;
		if( hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3' && hdr[3] >= 2 && hdr[3] != 0xff) {
			unsigned int len = id3v2_tag_size(hdr[3], tiny);
			if(len == -1){
				ret = DECODER_EOF;
				goto parse_exit;
			}
			if(len > 0)
				lseek(tiny->fd, len-10, SEEK_CUR);
			step = 4;
		} else if((mp3_flen = mp_get_mp3_header(hdr, &mp3_chans, &mp3_freq,
						&mpa_spf, &mpa_layer, &mpa_br)) > 0) {
			mp3_found = add_mp3_hdr(&mp3_hdrs, st_pos, mp3_chans, mp3_freq,
					mpa_spf, mpa_layer, mpa_br, mp3_flen);
			if (mp3_found){
				tiny->channels = mp3_found->mp3_chans;
				tiny->bps	= mp3_found->mpa_br;
				tiny->samplerate = mp3_found->mp3_freq;
				lseek(tiny->fd, mp3_found->frame_pos, SEEK_SET);
				DEBUG("frame start pos: %ld\n",mp3_found->frame_pos);
				ret = DECODER_OK;
				goto parse_exit;
			}
		}

		if(step < HDR_SIZE)
			memmove(hdr,&hdr[step],HDR_SIZE-step);

		rSize = read(tiny->fd, &hdr[HDR_SIZE - step], step);
		if (rSize < step && rSize >= 0 ) {
			ret = DECODER_EOF;
			goto parse_exit;
		} else if (rSize < 0) {
			ERROR("read file: %s", strerror(errno));
			ret = DECODER_UNRECOVERY;
			goto parse_exit;
		}
		n++;
	}

parse_exit:

	if(n == 30000)
		ret = DECODER_EOF;

	free_mp3_hdrs(&mp3_hdrs);
	if (mp3_found){
		free(mp3_found);
		mp3_found = NULL;
	}
	return ret;
}

static audio_output_t *set_audio(tiny_mp3_t *tiny, char *interface, int *quit)
{
	mad_decoder_t *decode = &tiny->decoder;
	audio_output_t *ao;
	ao_format_t fmt;
	int retval;

	ao = audio_get_output(interface);
	if (!ao) {
		ERROR("NOT found audio interface");
		return NULL;
	}
	ao->init(0, NULL);

	decode_zero_in(decode, tiny->fd);
	retval = parse_header(tiny, quit);

	switch (retval) {
		case DECODER_OK:
			fmt.channels = tiny->channels;
			fmt.bytes = 2; /* force 16bit */
			fmt.rate = tiny->samplerate;
			break;

		case DECODER_EOF:
			ERROR("NOT found mp3 header");
			ERROR("This is not a mp3 file");
		case DECODER_UNRECOVERY:
			return NULL;

		case DECODER_STOP:
			DEBUG("Stop in action");
			return NULL;
	}

	retval = ao->start(&fmt);
	if (retval < 0) {
		ao->deinit();
		return NULL;
	}

	return ao;
}

int tinymp3_play(char *mp3_file)
{
	audio_output_t *ao;
	struct stat stat;
	int retval;
	int err = 0;
	char *ao_type = "alsa";//default is oss

	//if (AUDIO_ALSA == get_audio_type())
	//ao_type = "alsa";

	if (!mp3_file || strlen(mp3_file) == 0) {
		ERROR("file name is empty");
		return -1;
	}

	tiny = malloc(sizeof(tiny_mp3_t));
	if (!tiny) {
		ERROR("Alloc mad_decoder: %s", strerror(errno));
		return -1;
	}

	memset(tiny, 0, sizeof(tiny_mp3_t));

	tiny->fd = open(mp3_file, O_RDONLY);
	if (tiny->fd < 0) {
		ERROR("Open media file: %s", strerror(errno));
		err = -1;
		goto err_open_file;
	}

	if (fstat(tiny->fd, &stat) == -1) {
		ERROR("Get file status: %s", strerror(errno));
		err = -1;
		goto err_fstat;
	}

	if (!S_ISREG(stat.st_mode) && !S_ISLNK(stat.st_mode)) {
		char type[32] = {0};
		get_file_type(&stat, type, sizeof(type));
		ERROR("\'%s\' Is a %s", mp3_file, type);
		err = -1;
		goto not_regular_file;
	}

	if (!stat.st_size) {
		ERROR("\'%s\' Is a empty file", mp3_file);
		err = -1;
		goto empty_file;
	}

	retval = play_device_try(ao_type);
	if (retval < 0) {
		ERROR("Audio device occupied, Can not play\n");
		err = -1;
		goto err_device_try;
	}

	retval = decode_init(&tiny->decoder);
	if (retval < 0) {
		err = -1;
		goto err_decode_init;
	}

	ao = set_audio(tiny, ao_type, &quit_flag);
	if (!ao){
		DEBUG("set_pcm failed");
		err = -1;
		goto err_set_audio;
	}

	//decode_zero_in(&tiny->decoder, tiny->fd);

	while (!quit_flag) {
		retval = decode_frame(&tiny->decoder, tiny->fd, &quit_flag);
		switch (retval) {
		case DECODER_OK:
			play_frame(&tiny->decoder, ao);
			break;

		case DECODER_UNRECOVERY:
			err = -1;
			DEBUG("Tinymp3 stop with unrecovery");
			goto err_decode_unrecovery;

		case DECODER_EOF:
			DEBUG("Tinymp3 stop without action %d", quit_flag);
			goto process_end;
			break;
		case DECODER_STOP:
			DEBUG("Tinymp3 stop with action %d", quit_flag);
			quit_flag = 1;
			goto process_end;
			break;
		}
	}

process_end://mark
	if(quit_flag)
		err = -255;

err_decode_unrecovery:
	ao->stop();
	ao->deinit();

err_set_audio:
	decode_deinit(&tiny->decoder);

err_decode_init:
err_device_try:
empty_file:
not_regular_file:
err_fstat:
	close(tiny->fd);

err_open_file:
	free(tiny);
	tiny= NULL;//mark
	quit_flag	= 0;
	
	return err;
}

void tinymp3_reset(void)
{
	tiny		= NULL;
	quit_flag	= 0;
}

void tinymp3_stop(void)
{
	if(tiny)
		quit_flag = 1;
}
