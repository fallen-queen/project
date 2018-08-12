#ifndef __ALSA_CTRL__
#define __ALSA_CTRL__

#include <alsa/asoundlib.h>

/**************************************************************************/
/*******************************DEFINITIONS********************************/

/**************************************************************************/
/***********************************TYPES**********************************/
typedef struct _ALSA_ENV{
	snd_pcm_t *handle;
	snd_pcm_format_t format;
	snd_pcm_uframes_t period_size;
	unsigned int channels;
	unsigned int rate;
	unsigned int start_delay;
} ALSA_ENV;

/**************************************************************************/
/***************************LOCAL FUNCTIONS********************************/

/**************************************************************************/
/*************************GLOBAL FUNCTIONS*********************************/
int alsa_set_params(ALSA_ENV *pEnv);

int alsa_xrun_recovery(snd_pcm_t *handle, int err);

#endif
