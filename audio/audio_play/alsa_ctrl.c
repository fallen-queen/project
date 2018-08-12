#include "alsa_ctrl.h"

/**************************************************************************/
/*******************************DEFINITIONS********************************/

/**************************************************************************/
/***********************************TYPES**********************************/

/**************************************************************************/
/***************************LOCAL FUNCTIONS********************************/

/**************************************************************************/
/*************************GLOBAL FUNCTIONS*********************************/
int alsa_set_params(ALSA_ENV *pEnv)
{
	int ret = 0;
	int err = 0;
	snd_pcm_t *handle = NULL;
	snd_pcm_hw_params_t *params = NULL;
	snd_pcm_sw_params_t *swparams = NULL;
	snd_pcm_uframes_t buffer_size, period_size;
	snd_pcm_uframes_t start_threshold, stop_threshold;
	unsigned int buffer_time, period_time;
	unsigned int rate;

	handle = pEnv->handle;

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);

	/* choose all parameters */
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		printf("no configurations available\n");
		ret = -1;
		goto err_leave;
	}

	/* set the selected read/write format */
	err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		printf("can not set access type\n");
		ret = -1;
		goto err_leave;
	}

	/* set the sample format */
	err = snd_pcm_hw_params_set_format(handle, params, pEnv->format);
	if (err < 0) {
		printf("can not set sample format\n");
		ret = -1;
		goto err_leave;
	}

	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, params, pEnv->channels);
	if (err < 0) {
		printf("can not set channels\n");
		ret = -1;
		goto err_leave;
	}

	/* set the stream rate */
	rate = pEnv->rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &pEnv->rate, 0);
	if (err < 0) {
		printf("can not set rate\n");
		ret = -1;
		goto err_leave;
	}
	if ((float)rate * 1.05 < pEnv->rate || (float)rate * 0.95 > pEnv->rate) {
		printf("rate is not accurate\n");
	}
	rate = pEnv->rate;

	/* get buffer time max */
	buffer_time = 0;
	period_time = 0;
	if (pEnv->period_size == 0) {
		err = snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
	}

	if (buffer_time > 500000) {
		buffer_time = 500000;   /* 500ms */
	}

	if (buffer_time > 0) {
		period_time = buffer_time / 4;
	}

	/* set period time or period size */
	if (period_time > 0) {
		err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
	}
	else {
		err = snd_pcm_hw_params_set_period_size_near(handle, params, &pEnv->period_size, 0);
	}
	if (err < 0) {
		printf("can not set period time or period size\n");
		ret = -1;
		goto err_leave;
	}

	/* set buffer time or buffer size */
	if (period_time > 0) {
		err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, 0);
	}
	else {
		buffer_size = pEnv->period_size * 4;
		err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_size);
	}
	if (err < 0) {
		printf("can not set buffer time or buffer size\n");
		ret = -1;
		goto err_leave;
	}

	/* set hw params */
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		printf("can not set alsa hw params\n");
		ret = -1;
		goto err_leave;
	}

	/* get period size */
	err = snd_pcm_hw_params_get_period_size(params, &period_size, 0);
	if (err < 0) {
		printf("can not get period size\n");
		ret = -1;
		goto err_leave;
	}
	pEnv->period_size = period_size;

	/* get buffer size */
	err = snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (err < 0) {
		printf("can not get buffer size\n");
		ret = -1;
		goto err_leave;
	}

	/* choose software params */
	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		printf("can not choose software params\n");
		ret = -1;
		goto err_leave;
	}

	/* set minimum avail size */
	err = snd_pcm_sw_params_set_avail_min(handle ,swparams, pEnv->period_size);
	if (err < 0) {
		printf("can not set avail min\n");
		ret = -1;
		goto err_leave;
	}

	/* set start_threshold */
	/* in milisecond -> divide 1000 */
	start_threshold = (double)rate * pEnv->start_delay / 1000;
	if (start_threshold < 1) {
		start_threshold = 1;
	}
	else if (start_threshold > buffer_size) {
		start_threshold = buffer_size;
	}
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
	if (err < 0) {
		printf("can not set start_threshold\n");
		ret = -1;
		goto err_leave;
	}

	/* set sw params */
	err = snd_pcm_sw_params(handle, swparams);
	if (err < 0) {
		printf("can not set sw params\n");
		ret = -1;
		goto err_leave;
	}
	
err_leave:
	return ret;
}

int alsa_xrun_recovery(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE) {
		printf("recovery from underrun\n");
		err = snd_pcm_prepare(handle);
	}
	else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			sleep(1);
		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0) {
				printf("can not recovery from suspend, prepare failed\n");
			}
		}
		return 0;
	}

	return err;
}

