#define TAG "TinaSoundControl"
#define CONFIG_TLOG_LEVEL OPTION_TLOG_LEVEL_CLOSE
#include <tina_log.h>

#include "tinasoundcontrol.h"
#include <pthread.h>
#include <sys/time.h>

namespace aw{
	int BLOCK_MODE = 0;
	int NON_BLOCK_MODE = 1;
	static int openSoundDevice(SoundCtrlContext* sc, bool isOpenForPlay ,int mode);
	static int closeSoundDevice(SoundCtrlContext* sc);
	static int setSoundDeviceParams(SoundCtrlContext* sc);

	static int openSoundDevice(SoundCtrlContext* sc , bool isOpenForPlay,int mode)
	{
		int ret = 0;
		TLOGD("openSoundDevice()\n");
		printf("openSoundDevice()\n");
		if(!sc->alsa_handler){
			if(isOpenForPlay){
				if((ret = snd_pcm_open(&sc->alsa_handler, "default",SND_PCM_STREAM_PLAYBACK ,mode))<0){
					TLOGE("open audio device failed:%s\n, errno = %d",strerror(errno),errno);
					if(errno == 16){//the device is busy,sleep 2 second and try again
						sleep(2);
						if((ret = snd_pcm_open(&sc->alsa_handler, "default",SND_PCM_STREAM_PLAYBACK ,mode))<0){
							TLOGE("open audio device failed:%s\n, errno = %d",strerror(errno),errno);
						}
					}
				}
			}else{
				if((ret = snd_pcm_open(&sc->alsa_handler, "default",SND_PCM_STREAM_CAPTURE ,mode))<0){
					TLOGE("open audio device failed:%s\n",strerror(errno));
				}
			}
		}else{
			TLOGD("the audio device has been opened\n");
		}
		return ret;
	}

	static int closeSoundDevice(SoundCtrlContext* sc)
	{
		int ret = 0;
		TLOGD("closeSoundDevice()\n");
		printf("closeSoundDevice()\n");
		if (sc->alsa_handler){
			if ((ret = snd_pcm_close(sc->alsa_handler)) < 0) 
	        {
	            TLOGE("snd_pcm_close failed:%s\n",strerror(errno));
			}
			else
			{
				sc->alsa_handler = NULL;
	            TLOGD("alsa-uninit: pcm closed\n");
			}
		}
		return ret;
	}

	static int setSoundDeviceParams(SoundCtrlContext* sc)
	{
		int ret = 0;
		TLOGD("setSoundDeviceParams()\n");
		if ((ret = snd_pcm_hw_params_malloc(&sc->alsa_hwparams)) < 0)
		{
			TLOGE("snd_pcm_hw_params_malloc failed:%s\n",strerror(errno));
			return ret;
		}

		if ((ret = snd_pcm_hw_params_any(sc->alsa_handler, sc->alsa_hwparams)) < 0) 
        {
		TLOGE("snd_pcm_hw_params_any failed:%s\n",strerror(errno));
		return ret;
        }
		
		if ((ret = snd_pcm_hw_params_set_access(sc->alsa_handler, sc->alsa_hwparams,
                    SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
        {
		TLOGE("snd_pcm_hw_params_set_access failed:%s\n",strerror(errno));
		return ret;
        }

		if ((ret = snd_pcm_hw_params_test_format(sc->alsa_handler, sc->alsa_hwparams,
                sc->alsa_format)) < 0) 
        {
            TLOGE("MSGTR_AO_ALSA_FormatNotSupportedByHardware");
            sc->alsa_format = SND_PCM_FORMAT_S16_LE;
        }
		
		if ((ret = snd_pcm_hw_params_set_format(sc->alsa_handler, sc->alsa_hwparams,
                    sc->alsa_format)) < 0) 
        {
            TLOGE("snd_pcm_hw_params_set_format failed:%s\n",strerror(errno));
            return ret;
        }

        if ((ret = snd_pcm_hw_params_set_channels_near(sc->alsa_handler,
                sc->alsa_hwparams, &sc->nChannelNum)) < 0) {
            TLOGE("snd_pcm_hw_params_set_channels_near failed:%s\n",strerror(errno));
            return ret;
        }

		if ((ret = snd_pcm_hw_params_set_rate_near(sc->alsa_handler, sc->alsa_hwparams,
                &sc->nSampleRate, NULL)) < 0) {    
            TLOGE("snd_pcm_hw_params_set_rate_near failed:%s\n",strerror(errno));
            return ret;
        }

		sc->bytes_per_sample = snd_pcm_format_physical_width(sc->alsa_format) / 8;
	    sc->bytes_per_sample *= sc->nChannelNum;
		sc->alsa_fragcount = 8;
        sc->chunk_size = 2048;
		
		if ((ret = snd_pcm_hw_params_set_period_size_near(sc->alsa_handler,
				sc->alsa_hwparams, &sc->chunk_size, NULL)) < 0) {
			TLOGE("MSGTR_AO_ALSA_UnableToSetPeriodSize");
			return ret;
		} else {
			TLOGD("alsa-init: chunksize set to %d\n", sc->chunk_size);
		}
		TLOGD("before snd_pcm_hw_params_set_periods_near:alsa-init: fragcount=%d\n", sc->alsa_fragcount);
        if ((ret = snd_pcm_hw_params_set_periods_near(sc->alsa_handler,
                sc->alsa_hwparams, (unsigned int*)&sc->alsa_fragcount, NULL)) < 0)
        {
            TLOGE("MSGTR_AO_ALSA_UnableToSetPeriods");
            return ret;
        } else {
            TLOGD("alsa-init: fragcount=%d\n", sc->alsa_fragcount);
        }

		if ((ret = snd_pcm_hw_params(sc->alsa_handler, sc->alsa_hwparams)) < 0) {
            TLOGE("snd_pcm_hw_params failed:%s\n",strerror(errno));
            return ret;
        }

		snd_pcm_hw_params_free(sc->alsa_hwparams);

		sc->alsa_can_pause = snd_pcm_hw_params_can_pause(sc->alsa_hwparams);

		TLOGD("setSoundDeviceParams():sc->alsa_can_pause = %d\n",sc->alsa_can_pause);

		return ret;

	}

	SoundCtrl* TinaSoundDeviceInit(void* pAudioSink){
		SoundCtrlContext* s;
	    s = (SoundCtrlContext*)malloc(sizeof(SoundCtrlContext));
		TLOGD("TinaSoundDeviceInit()\n");
	    if(s == NULL)
	    {
	        TLOGE("malloc memory fail.\n");
	        return NULL;
	    }
	    memset(s, 0, sizeof(SoundCtrlContext));
		s->alsa_access_type = SND_PCM_ACCESS_RW_INTERLEAVED;
		s->nSampleRate = 44100;
		s->nChannelNum = 2;
		s->alsa_format = SND_PCM_FORMAT_S16_LE;
		s->alsa_can_pause = 0;
		s->sound_status = STATUS_STOP;
		pthread_mutex_init(&s->mutex, NULL);
		return (SoundCtrl*)s;
	}

	void TinaSoundDeviceRelease(SoundCtrl* s){
		SoundCtrlContext* sc;
	sc = (SoundCtrlContext*)s;
		pthread_mutex_lock(&sc->mutex);
		TLOGD("TinaSoundDeviceRelease()\n");
		if(sc->sound_status != STATUS_STOP){
			closeSoundDevice(sc);
		}
		pthread_mutex_unlock(&sc->mutex);
	pthread_mutex_destroy(&sc->mutex);
		free(sc);
	sc = NULL;
	}

	void TinaSoundDeviceSetFormat(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum){
		SoundCtrlContext* sc;
	sc = (SoundCtrlContext*)s;
		pthread_mutex_lock(&sc->mutex);
		TLOGD("TinaSoundDeviceSetFormat(),sc->sound_status == %d\n",sc->sound_status);
		if(sc){
			if(sc->sound_status == STATUS_STOP){
				TLOGD("TinaSoundDeviceSetFormat()\n");
				sc->nSampleRate = nSampleRate;
				sc->nChannelNum = nChannelNum;
				sc->alsa_format = SND_PCM_FORMAT_S16_LE;
				sc->bytes_per_sample = snd_pcm_format_physical_width(sc->alsa_format) / 8;
			sc->bytes_per_sample *= nChannelNum;
				TLOGD("TinaSoundDeviceSetFormat()>>>sample_rate:%d,channel_num:%d,sc->bytes_per_sample:%d\n",
					nSampleRate,nChannelNum,sc->bytes_per_sample);
			}
		}
		else{
			TLOGE("error:sc is null !!!\n");
		}
		pthread_mutex_unlock(&sc->mutex);
	}

	int TinaSoundDeviceStart(SoundCtrl* s){
		SoundCtrlContext* sc;
	sc = (SoundCtrlContext*)s;
		pthread_mutex_lock(&sc->mutex);
		int ret = 0;
		TLOGD("TinaSoundDeviceStart(): sc->sound_status = %d\n",sc->sound_status);
		printf("TinaSoundDeviceStart()\n");
		if(sc->sound_status == STATUS_START){
			TLOGD("Sound device already start.\n");
			pthread_mutex_unlock(&sc->mutex);
			return ret;
		}else if(sc->sound_status == STATUS_PAUSE){
			if(snd_pcm_state(sc->alsa_handler) == SND_PCM_STATE_SUSPENDED){
				TLOGD("MSGTR_AO_ALSA_PcmInSuspendModeTryingResume\n");
				while((ret = snd_pcm_resume(sc->alsa_handler)) == -EAGAIN){
					sleep(1);
				}
			}
			if(sc->alsa_can_pause){
				if((ret = snd_pcm_pause(sc->alsa_handler, 0))<0){
					TLOGE("snd_pcm_pause failed:%s\n",strerror(errno));
					pthread_mutex_unlock(&sc->mutex);
			return ret;
				}
			}else{
				if ((ret = snd_pcm_prepare(sc->alsa_handler)) < 0) 
				{
					TLOGE("snd_pcm_prepare failed:%s\n",strerror(errno));
					pthread_mutex_unlock(&sc->mutex);
					return ret;
				}
			}
			sc->sound_status = STATUS_START;
		}
		else if(sc->sound_status == STATUS_STOP){
			sc->alsa_fragcount = 8;
            sc->chunk_size = 2048;//1024;
			ret = openSoundDevice(sc, true, BLOCK_MODE);
			TLOGD("after openSoundDevice() ret = %d\n",ret);
			if(ret >= 0){
				ret = setSoundDeviceParams(sc);
				sc->sound_status = STATUS_START;
			}
		}
		pthread_mutex_unlock(&sc->mutex);
		return ret;
	}

	int TinaSoundDeviceStop(SoundCtrl* s){
		int ret = 0;
		SoundCtrlContext* sc;
	sc = (SoundCtrlContext*)s;
		pthread_mutex_lock(&sc->mutex);
		TLOGD("TinaSoundDeviceStop():sc->sound_status = %d\n",sc->sound_status);
		if(sc->sound_status == STATUS_STOP)
	    {
	        TLOGD("Sound device already stopped.\n");
			pthread_mutex_unlock(&sc->mutex);
			return ret;
	    }else{
		if ((ret = snd_pcm_drop(sc->alsa_handler)) < 0)
		    {
		        TLOGE("MSGTR_AO_ALSA_PcmPrepareError");
				pthread_mutex_unlock(&sc->mutex);
				return ret;
			}
			if ((ret = snd_pcm_prepare(sc->alsa_handler)) < 0) 
		    {
		        TLOGE("MSGTR_AO_ALSA_PcmPrepareError");
				pthread_mutex_unlock(&sc->mutex);
				return ret;
			}
			ret = closeSoundDevice(sc);
			sc->sound_status = STATUS_STOP;	
		}
		pthread_mutex_unlock(&sc->mutex);
		return ret;
	}

	int TinaSoundDevicePause(SoundCtrl* s){
		SoundCtrlContext* sc;
	sc = (SoundCtrlContext*)s;
		pthread_mutex_lock(&sc->mutex);
		int ret = 0;
		TLOGD("TinaSoundDevicePause(): sc->sound_status = %d\n",sc->sound_status);
		if(sc->sound_status == STATUS_START){
			if(sc->alsa_can_pause){
				TLOGD("alsa can pause,use snd_pcm_pause\n");
				ret = snd_pcm_pause(sc->alsa_handler, 1);
				if(ret<0){
					TLOGE("snd_pcm_pause failed:%s\n",strerror(errno));
					pthread_mutex_unlock(&sc->mutex);
			return ret;
				}
			}else{
				TLOGD("alsa can not pause,use snd_pcm_drop\n");
				if ((ret = snd_pcm_drop(sc->alsa_handler)) < 0) 
				{
					TLOGE("snd_pcm_drop failed:%s\n",strerror(errno));
					pthread_mutex_unlock(&sc->mutex);
					return ret;
				}
			}
			sc->sound_status = STATUS_PAUSE;
		}else{
			TLOGD("TinaSoundDevicePause(): pause in an invalid status\n",sc->sound_status);
		}
		pthread_mutex_unlock(&sc->mutex);
		return ret;
	}

	int TinaSoundDeviceWrite(SoundCtrl* s, void* pData, int nDataSize){
		int ret;
	SoundCtrlContext* sc;
	sc = (SoundCtrlContext*)s;
		//TLOGD("TinaSoundDeviceWrite:sc->bytes_per_sample = %d\n",sc->bytes_per_sample);
		if(sc->bytes_per_sample == 0){
			sc->bytes_per_sample = 4;
		}
		if(sc->sound_status == STATUS_STOP || sc->sound_status == STATUS_PAUSE)
	    {
	        return ret;
	    }
		//TLOGD("TinaSoundDeviceWrite>>> pData = %p , nDataSize = %d\n",pData,nDataSize);
		int num_frames = nDataSize / sc->bytes_per_sample;
		snd_pcm_sframes_t res = 0;

		if (!sc->alsa_handler) 
	    {
	        TLOGE("MSGTR_AO_ALSA_DeviceConfigurationError");
			return ret;
		}

		if (num_frames == 0){
			TLOGE("num_frames == 0");
			return ret;
		}
		
		do {
			res = snd_pcm_writei(sc->alsa_handler, pData, num_frames);
			if (res == -EINTR) 
	        {
				/* nothing to do */
				res = 0;
			} else if (res == -ESTRPIPE) 
			{ /* suspend */
	            TLOGD("MSGTR_AO_ALSA_PcmInSuspendModeTryingResume\n");
				while ((res = snd_pcm_resume(sc->alsa_handler)) == -EAGAIN)
					sleep(1);
			}
			if (res < 0) 
	        {
	            TLOGE("MSGTR_AO_ALSA_WriteError\n");
				if ((res = snd_pcm_prepare(sc->alsa_handler)) < 0) 
	            {
	                TLOGE("MSGTR_AO_ALSA_PcmPrepareError\n");
					return res;
				}
			}
		} while (res == 0);
		return res < 0 ? res : res * sc->bytes_per_sample;
	}

	int TinaSoundDeviceReset(SoundCtrl* s){
		TLOGD("TinaSoundDeviceReset()\n");
		return TinaSoundDeviceStop(s);
	}

	int TinaSoundDeviceGetCachedTime(SoundCtrl* s){
		int ret = 0;
		SoundCtrlContext* sc;
		sc = (SoundCtrlContext*)s;	 
		//TLOGD("TinaSoundDeviceGetCachedTime()\n");
		if (sc->alsa_handler) 
		{
			snd_pcm_sframes_t delay;
			//notify:snd_pcm_delay means the cache has how much data(the cache has been filled with pcm data),
			//snd_pcm_avail_update means the free cache,  
			if ((ret = snd_pcm_delay(sc->alsa_handler, &delay)) < 0){
				TLOGE("TinaSoundDeviceGetCachedTime(),ret = %d , delay = %d\n",ret,delay);
				return ret;
			}
			//printf("TinaSoundDeviceGetCachedTime(),snd_pcm_delay>>> delay = %d\n",delay);
			//delay = snd_pcm_avail_update(sc->alsa_handler);
			//printf("TinaSoundDeviceGetCachedTime(), snd_pcm_avail_update >>> delay = %d\n",delay);
			if (delay < 0) {
				/* underrun - move the application pointer forward to catch up */
#if SND_LIB_VERSION >= 0x000901 /* snd_pcm_forward() exists since 0.9.0rc8 */
				snd_pcm_forward(sc->alsa_handler, -delay);
#endif
				delay = 0;
			}
			ret = ((int)((float) delay * 1000000 / (float) sc->nSampleRate));
		}
		return ret;
	}

	SoundCtrl* TinaSoundDeviceInit_raw(void* raw_data,void* hdeccomp,RawCallback callback){
		return NULL;
	}

	void TinaSoundDeviceRelease_raw(SoundCtrl* s){

	}

	void TinaSoundDeviceSetFormat_raw(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum){
		
	}

	int TinaSoundDeviceStart_raw(SoundCtrl* s){
		return 0;
	}

	int TinaSoundDeviceStop_raw(SoundCtrl* s){
		return 0;
	}

	int TinaSoundDevicePause_raw(SoundCtrl* s){
		return 0;
	}

	int TinaSoundDeviceWrite_raw(SoundCtrl* s, void* pData, int nDataSize){
		return 0;
	}

	int TinaSoundDeviceReset_raw(SoundCtrl* s){
		return 0;
	}

	int TinaSoundDeviceGetCachedTime_raw(SoundCtrl* s){
		return 0;
	}

	int TinaSoundDeviceSetVolume(SoundCtrl* s, float volume){
		return 0;
	}

	int TinaSoundDeviceGetVolume(SoundCtrl* s, float *volume){
		return 0;
	}

	int TinaSoundDeviceSetCallback(SoundCtrl* s, SndCallback callback, void* pUserData){
		return 0;
	}

	
	
}
