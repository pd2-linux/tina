/*
 * mp3player.h
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

#ifndef __TINY_MP3_H__
#define __TINY_MP3_H__

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <mad.h>

typedef struct mad_decoder_s {
	struct mad_synth	synth;
	struct mad_stream	stream;
	struct mad_frame	frame;

	void		*dbuf;
	int		buf_size;
	int		in_buf_len;
	int		eof;
} mad_decoder_t;

typedef struct {
	int		fd;
	mad_decoder_t	decoder;

	int		channels;
	int		bps;
	int		samplerate;
} tiny_mp3_t;

extern int tinymp3_play(char *mp3_file);

extern void tinymp3_reset(void);

extern void tinymp3_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* __TINY_MP3_H__ */
