#include <stdio.h>
#include "alsa_ctrl.h"
#include "wav_lib.h"

/**************************************************************************/
/*******************************DEFINITIONS********************************/

/**************************************************************************/
/***********************************TYPES**********************************/
typedef struct _AUDIO_PLAY_CONTEXT {
	ALSA_ENV alsa_env;
	unsigned int data_size;
	unsigned int period_buffer_size;
	void *period_buffer;
} AUDIO_PLAY_CONTEXT;

/**************************************************************************/
/***************************LOCAL FUNCTIONS********************************/
static int audio_play_init(AUDIO_PLAY_CONTEXT *play_ctx)
{
	int ret = 0;
	WAVE_HEADER wavHdr;

	/* read wave file header */
	if (read_wav_file("test.wav", &wavHdr, sizeof(WAVE_HEADER), 0) == false) {
		printf("fail to read wave file header\n");
		ret = -1;
		goto err_leave;
	}

	/* parse wave file header */
	if (parse_wav_header(&wavHdr) == false) {
		printf("wrong wave file header\n");
		ret = -1;
		goto err_leave;
	}

	/* open alsa interface */
	if (snd_pcm_open(&play_ctx->alsa_env.handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		printf("fail to open pcm device\n");
		ret = -1;
		goto err_leave;
	}

	/* init alsa parameters */
	play_ctx->alsa_env.channels = wavHdr.FmtBlk.wavFormat.Channels;
	play_ctx->alsa_env.rate = wavHdr.FmtBlk.wavFormat.SamplesPerSec;
	play_ctx->alsa_env.format = SND_PCM_FORMAT_S16_LE;
	play_ctx->alsa_env.start_delay = 200;
	play_ctx->data_size = wavHdr.DataBlk.DataSize;
	printf("channels = %d, rate = %d, data_size = %d\n", play_ctx->alsa_env.channels,
		play_ctx->alsa_env.rate, play_ctx->data_size);

	/* set alsa parameters */
	if (alsa_set_params(&play_ctx->alsa_env) < 0) {
		printf("fail to open alsa interface\n");
		ret = -1;
		goto err_leave;
	}
	play_ctx->period_buffer_size = play_ctx->alsa_env.period_size * play_ctx->alsa_env.channels * 2;
	play_ctx->data_size = play_ctx->data_size / play_ctx->period_buffer_size * play_ctx->period_buffer_size;

	/* malloc memory */
	if (play_ctx->period_buffer) {
		free(play_ctx->period_buffer);
		play_ctx->period_buffer = NULL;
	}
	play_ctx->period_buffer = malloc(play_ctx->period_buffer_size);
	if (!play_ctx->period_buffer) {
		ret = -1;
		goto err_leave;
	}

	return ret;

err_leave:
	if (play_ctx->alsa_env.handle) {
		snd_pcm_close(play_ctx->alsa_env.handle);
		play_ctx->alsa_env.handle = NULL;
	}
	if (play_ctx->period_buffer) {
		free(play_ctx->period_buffer);
		play_ctx->period_buffer = NULL;
	}
	return ret;
}

static void audio_play_stop(AUDIO_PLAY_CONTEXT *play_ctx)
{
	if (play_ctx->alsa_env.handle) {
		snd_pcm_close(play_ctx->alsa_env.handle);
		play_ctx->alsa_env.handle = NULL;
	}
	if (play_ctx->period_buffer) {
		free(play_ctx->period_buffer);
		play_ctx->period_buffer = NULL;
	}
}

static void audio_play(AUDIO_PLAY_CONTEXT *play_ctx, void *buffer, int size)
{
	snd_pcm_t *handle = NULL;
	int frames_remains;
	int frames_snt;
	int frames;

	handle = play_ctx->alsa_env.handle;

	frames_remains = size / play_ctx->alsa_env.channels / 2;
	while (frames_remains > 0) {
		frames_snt = (frames_remains > play_ctx->alsa_env.period_size) ? play_ctx->alsa_env.period_size : frames_remains;
		frames = snd_pcm_writei(handle, buffer, frames_snt);
		if (frames < 0) {
			alsa_xrun_recovery(handle, frames);
		}
		else {
			frames_remains -= frames;
		}
	}
	
}

/**************************************************************************/
/*************************GLOBAL FUNCTIONS*********************************/


int main(int argc, char **argv)
{
	unsigned int snd_cnt = 0;
	char *filename = NULL;
	AUDIO_PLAY_CONTEXT play_ctx;
	memset(&play_ctx, 0, sizeof(play_ctx));

	if (argc < 2) {
		printf("please input wav file name you wanna play!\n");
		return -1;
	}
	filename = argv[1];

	if (audio_play_init(&play_ctx) != 0) {
		printf("fail to init audio play!\n");
		return -1;
	}
	for(snd_cnt = 0; snd_cnt < play_ctx.data_size; snd_cnt += play_ctx.period_buffer_size) {
		if (read_wav_file(filename, play_ctx.period_buffer, play_ctx.period_buffer_size, sizeof(WAVE_HEADER) + snd_cnt)) {
			audio_play(&play_ctx, play_ctx.period_buffer, play_ctx.period_buffer_size);
		}
		usleep(36000);
	}
	audio_play_stop(&play_ctx);

	return 0;
}
