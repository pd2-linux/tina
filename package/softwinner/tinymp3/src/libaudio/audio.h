/*
	audio.h
 */

#ifndef _AUDIO_H__
#define _AUDIO_H__

/**
 * ao_format_t
 * @channels:
 * @bits:
 * @format:
 * @rate:
 */
typedef struct {
	int	channels;	/* number of audio channels */
	int	bytes;		/* bytes per sample */
	int	format;		/* pcm format */
	int	rate;		/* samples per second */
} ao_format_t;

/**
 * audio_output_t
 */
typedef struct {
	char	*name;

	int (*init)(int argc, char **argv);
	void (*deinit)(void);

	int (*start)(ao_format_t *fmt);
	void (*play)(short *buf, int samples);
	void (*stop)(void);

	void (*help)(void);
	void (*volume)(double vol);
	int (*dev_try)(void);
	int (*get_space)(void);
} audio_output_t;

/**
 * audio_get_output -
 */
extern audio_output_t *audio_get_output(char *name);

/**
 * audio_ls_outputs -
 */
extern void audio_ls_outputs(void);

#endif /* _AUDIO_H__ */
