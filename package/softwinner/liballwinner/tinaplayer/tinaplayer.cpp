#define LOG_TAG "TinaPlayer"
#include "tinaplayer.h"
#include <string.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include "awplayer.h"
#include "tinasoundcontrol.h"
#include "log.h"

#define SAVE_PCM_DATA 0

#if SAVE_PCM_DATA
	FILE* savaPcmFd = NULL;
#endif

namespace aw{

	SoundCtrl* gSoundCtrl = NULL;
	static SoundCtrl* _SoundDeviceInit(void* pAudioSink){
		logd("_SoundDeviceInit()");
		SoundCtrl* soundCtrl = NULL;
		soundCtrl = TinaSoundDeviceInit(pAudioSink);
		if(soundCtrl == NULL){
			loge(" _SoundDeviceInit(),ERR:soundCtrl == NULL");
		}
		return soundCtrl;
	}

	static void _SoundDeviceRelease(SoundCtrl* s){
		logd(" _SoundDeviceRelease()");
		TinaSoundDeviceRelease(s);
	}

	static void _SoundDeviceSetFormat(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum){
		logd(" _SoundDeviceSetFormat()");
		TinaSoundDeviceSetFormat(s, nSampleRate, nChannelNum);
	}

	static int _SoundDeviceStart(SoundCtrl* s){
		logd(" _SoundDeviceStart()");
		//system("amixer cset numid=27 0");
		return TinaSoundDeviceStart(s);
	}

	static int _SoundDeviceStop(SoundCtrl* s){
		logd(" _SoundDeviceStop()");
		return TinaSoundDeviceStop(s);
	}

	static int _SoundDevicePause(SoundCtrl* s){
		logd(" _SoundDevicePause()");
		return TinaSoundDevicePause(s);
	}

	static int _SoundDeviceWrite(SoundCtrl* s, void* pData, int nDataSize){
		//logd(" _SoundDeviceWrite(),nDataSize = %d",nDataSize);
		int ret = TinaSoundDeviceWrite(s, pData, nDataSize);
		#if SAVE_PCM_DATA
			if(savaPcmFd!=NULL){
				int write_ret = fwrite(pData, 1, nDataSize, savaPcmFd);
				//logd("PCM write_ret = %d",write_ret);
				if(write_ret <= 0){
					loge("_SoundDeviceWrite error,err str: %s",strerror(errno));
				}
			}
		#endif
		return ret;
	}

	static int _SoundDeviceReset(SoundCtrl* s){
		logd(" _SoundDeviceReset()");
		return TinaSoundDeviceReset(s);
	}

	static int _SoundDeviceGetCachedTime(SoundCtrl* s){
		//logd(" _SoundDeviceGetCachedTime()");
		return TinaSoundDeviceGetCachedTime(s);
	}

	
	static SoundCtrl* _SoundDeviceInit_raw(void* raw_data,void* hdeccomp,RawCallback callback){
		return TinaSoundDeviceInit_raw(raw_data,hdeccomp,callback);
	}
	
	static void _SoundDeviceRelease_raw(SoundCtrl* s){
		TinaSoundDeviceRelease_raw(s);
	}

	static void _SoundDeviceSetFormat_raw(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum){
		TinaSoundDeviceSetFormat_raw(s, nSampleRate, nChannelNum);
	}

	static int _SoundDeviceStart_raw(SoundCtrl* s){
		return TinaSoundDeviceStart_raw(s);
	}

	static int _SoundDeviceStop_raw(SoundCtrl* s){
		return TinaSoundDeviceStop_raw(s);
	}

	static int _SoundDevicePause_raw(SoundCtrl* s){
		return TinaSoundDevicePause_raw(s);
	}

	static int _SoundDeviceWrite_raw(SoundCtrl* s, void* pData, int nDataSize){
		return TinaSoundDeviceWrite_raw(s, pData, nDataSize);
	}

	static int _SoundDeviceReset_raw(SoundCtrl* s){
		return TinaSoundDeviceReset_raw(s);
	}

	static int _SoundDeviceGetCachedTime_raw(SoundCtrl* s){
		return TinaSoundDeviceGetCachedTime_raw(s);
	}

	static int _SoundDeviceSetVolume(SoundCtrl* s, float volume){
		logd(" _SoundDeviceSetVolume()");
		return TinaSoundDeviceSetVolume(s, volume);
	}

	static int _SoundDeviceGetVolume(SoundCtrl* s, float *volume){
		logd(" _SoundDeviceGetVolume()");
		return TinaSoundDeviceGetVolume(s, volume);
	}

	static int _SoundDeviceSetCallback (SoundCtrl* s, SndCallback callback, void* pUserData){
		logd(" _SoundDeviceSetCallback()");
		return TinaSoundDeviceSetCallback(s, callback, pUserData);
	}

	static SoundControlOpsT gSoundControl;
	
	//* a callback for awplayer.
	void CallbackForAwPlayer(void* pUserData, int msg, int param0, void* param1)
	{
		int app_msg = 0;
		TinaPlayer* p = NULL;
		p = (TinaPlayer*)pUserData;
	    switch(msg)
	    {
	        case NOTIFY_NOT_SEEKABLE:
	        {
				logd(" ***NOTIFY_NOT_SEEKABLE***");
				app_msg = TINA_NOTIFY_NOT_SEEKABLE;
	            break;
	        }
	        
	        case NOTIFY_ERROR:
	        {
				logd(" ****NOTIFY_ERROR***");
				app_msg = TINA_NOTIFY_ERROR;
	            break;
	        }
	            
	        case NOTIFY_PREPARED:
	        {
				logd(" ***NOTIFY_PREPARED***");
				app_msg = TINA_NOTIFY_PREPARED;
	            break;
	        }
	            
	        case NOTIFY_BUFFERRING_UPDATE:
	        {
	            //int nBufferedFilePos;
	            //int nBufferFullness;
	            
	            //nBufferedFilePos = param0 & 0x0000ffff;
	            //nBufferFullness  = (param0>>16) & 0x0000ffff;
	            //TLOGD("info: buffer %d percent of the media file, buffer fullness = %d percent.\n", 
	            //    nBufferedFilePos, nBufferFullness);
	            //TLOGD(" NOTIFY_BUFFERRING_UPDATE\n");
				//app_msg = TINA_NOTIFY_BUFFERRING_UPDATE;
	            break;
	        }
	            
	        case NOTIFY_PLAYBACK_COMPLETE:
	        {
	            logd(" ****NOTIFY_PLAYBACK_COMPLETE****");
				if(p){
					if(p->mLoop == 0){
						logd(" ****NOTIFY_PLAYBACK_COMPLETE,close the sound card****");
						app_msg = TINA_NOTIFY_PLAYBACK_COMPLETE;
					}
				}
	            break;
	        }
	            
	        case NOTIFY_RENDERING_START:
	        {
	            logd(" NOTIFY_RENDERING_START");
				app_msg = TINA_NOTIFY_RENDERING_START;
	            break;
	        }
	        
	        case NOTIFY_SEEK_COMPLETE:
	        {
	            logd(" NOTIFY_SEEK_COMPLETE****");
				app_msg = TINA_NOTIFY_SEEK_COMPLETE;
				if(param1 != NULL){
					logd(" seek to result = %d",((int*)param1)[0]);
					logd(" seek to %d ms",((int*)param1)[1]);
				}
	            break;
	        }

	        case NOTIFY_VIDEO_FRAME:
	        {
				//in this version,NOTIFY_VIDEO_FRAME can not be called backed,20160320
				
				//VideoPicData* videodata = (VideoPicData*)param1;
				//if(videodata)
				//{
				//	if(videodata->ePixelFormat == VIDEO_PIXEL_FORMAT_YUV_MB32_420)
				//	{
				//		char filename[024];
			    //    	sprintf(filename, "/mnt/UDISK/mb32_%d.dat", pDemoPlayer->mVideoFrameNum);
			    //    	FILE* outFp = fopen(filename, "wb");
			    //    	if(outFp != NULL)
				//	    {
				//	    	int height64Align = (videodata->nHeight + 63)& ~63;
				//	    	fwrite(videodata->pData0, videodata->nWidth*videodata->nHeight, 1, outFp);
				//	    	fwrite(videodata->pData1, videodata->nWidth*height64Align/2, 1, outFp);
				//	    	fclose(outFp);
				//	    }
				//	}
			    //}	
			    //app_msg = TINA_NOTIFY_VIDEO_FRAME;
				//TLOGD(" NOTIFY_VIDEO_FRAME\n");
	        	break;
	        }

	        case NOTIFY_AUDIO_FRAME:
	        {
				//in this version,NOTIFY_AUDIO_FRAME can not be called backed,20160320
				
				//TLOGD(" NOTIFY_AUDIO_FRAME\n");
				//AudioPcmData* pcmData = (AudioPcmData*)param1;
				//#if SAVE_PCM_DATA
				//	if(savaPcmFd!=NULL){
				//		int write_ret = fwrite(pcmData->pData, 1, pcmData->nSize, savaPcmFd);
						//TLOGD("PCM write_ret = %d\n",write_ret);
				//		if(write_ret <= 0){
				//			TLOGD("err str: %s\n",strerror(errno));
				//		}
				//	}
				//#endif
				//app_msg = TINA_NOTIFY_AUDIO_FRAME;
	        	break;
	        }

	        case NOTIFY_VIDEO_PACKET:
	        {
	        	//DemuxData* videoData = (DemuxData*)param1;
				//logd("videoData pts: %lld", videoData->nPts);
				//static int frame = 0;
				//if(frame == 0)
				//{
				//	FILE* outFp = fopen("/mnt/UDISK/video.jpg", "wb");
		        //	if(videoData->nSize0)
		        //	{
		        //		fwrite(videoData->pData0, 1, videoData->nSize0, outFp);
		        //	}
		        //	if(videoData->nSize1)
		        //	{
		        //		fwrite(videoData->pData1, 1, videoData->nSize1, outFp);
		        //	}
		        //	fclose(outFp);
		        //	frame ++;
	        	//}
	        	//TLOGD(" NOTIFY_VIDEO_PACKET\n");
	        	//app_msg = TINA_NOTIFY_VIDEO_PACKET;
	        	break;
	        }

	        case NOTIFY_AUDIO_PACKET:
	        {
	        	//DemuxData* audioData = (DemuxData*)param1;
				//logd("audio pts: %lld", audioData->nPts);
				//static int audioframe = 0;
				//if(audioframe == 0)
				//{
				//	FILE* outFp = fopen("/mnt/UDISK/audio.pcm", "wb");
		        //	if(audioData->nSize0)
		        //	{
		        //		fwrite(audioData->pData0, 1, audioData->nSize0, outFp);
		        //	}
		        //	if(audioData->nSize1)
		        //	{
		        //		fwrite(audioData->pData1, 1, audioData->nSize1, outFp);
		        //	}
		        //	fclose(outFp);
		        //	audioframe ++;
	        	//}
	        	//TLOGD(" NOTIFY_AUDIO_PACKET\n");
	        	//app_msg = TINA_NOTIFY_AUDIO_PACKET;
	        	break;
	        	
	        }
	        
	        default:
	        {
	            logd(" warning: unknown callback from AwPlayer");
	            break;
	        }
	    }
		if(app_msg != 0){
			if(p){
				p->callbackToApp(app_msg, param0, param1);
			}
		}
	}
	TinaPlayer::TinaPlayer()
	{
		logd(" TinaPlayer() contructor begin");
		mLoop = 0;
		mNotifier = NULL;
		mUserData = NULL;
		mPlayer = (void*)new AwPlayer();
		initSoundControlOpsT();
		((AwPlayer*)mPlayer)->setControlOps(NULL, &gSoundControl);
		logd(" TinaPlayer() contructor finish");
	}
	
	TinaPlayer::~TinaPlayer()
	{
		logd(" ~TinaPlayer() contructor begin");
		if(((AwPlayer*)mPlayer) != NULL){
			delete ((AwPlayer*)mPlayer);
    		mPlayer = NULL;
		}
		logd(" ~TinaPlayer() contructor finish");
	}

	void TinaPlayer::initSoundControlOpsT(){
		gSoundControl.SoundDeviceInit = _SoundDeviceInit;
		gSoundControl.SoundDeviceRelease = _SoundDeviceRelease;
		gSoundControl.SoundDeviceSetFormat = _SoundDeviceSetFormat;
		gSoundControl.SoundDeviceStart = _SoundDeviceStart;
		gSoundControl.SoundDeviceStop = _SoundDeviceStop;
		gSoundControl.SoundDevicePause = _SoundDevicePause;
		gSoundControl.SoundDeviceWrite = _SoundDeviceWrite;
		gSoundControl.SoundDeviceReset = _SoundDeviceReset;
		gSoundControl.SoundDeviceGetCachedTime = _SoundDeviceGetCachedTime;
		gSoundControl.SoundDeviceInit_raw = _SoundDeviceInit_raw;
		gSoundControl.SoundDeviceRelease_raw = _SoundDeviceRelease_raw;
		gSoundControl.SoundDeviceSetFormat_raw = _SoundDeviceSetFormat_raw;
		gSoundControl.SoundDeviceStart_raw = _SoundDeviceStart_raw;
		gSoundControl.SoundDeviceStop_raw = _SoundDeviceStop_raw;
		gSoundControl.SoundDevicePause_raw = _SoundDevicePause_raw;
		gSoundControl.SoundDeviceWrite_raw = _SoundDeviceWrite_raw;
		gSoundControl.SoundDeviceReset_raw = _SoundDeviceReset_raw;
		gSoundControl.SoundDeviceGetCachedTime_raw = _SoundDeviceGetCachedTime_raw;
		gSoundControl.SoundDeviceSetVolume = _SoundDeviceSetVolume;
		gSoundControl.SoundDeviceGetVolume = _SoundDeviceGetVolume;
		gSoundControl.SoundDeviceSetCallback = _SoundDeviceSetCallback;
	}
	
	int TinaPlayer::initCheck()
	{
		return ((AwPlayer*)mPlayer)->initCheck();
	}
	
	int TinaPlayer::setNotifyCallback(NotifyCallback notifier, void* pUserData)
	{
		mNotifier = notifier;
		mUserData = pUserData;
		return ((AwPlayer*)mPlayer)->setNotifyCallback(CallbackForAwPlayer, (void*)this);
	}
	
	int TinaPlayer::setDataSource(const char* pUrl, const map<string, string>* pHeaders)
	{
		return ((AwPlayer*)mPlayer)->setDataSource(pUrl, pHeaders);
	}
	
	
	int TinaPlayer::prepareAsync()
	{
		pid_t tid = syscall(SYS_gettid);
		logd("TinaPlayer::prepareAsync() begin,tid = %d",tid);
		int ret = ((AwPlayer*)mPlayer)->prepareAsync();
		logd("TinaPlayer::prepareAsync() finish,tid = %d",tid);
		return ret;
	}
	
	int TinaPlayer::prepare()
	{
		return ((AwPlayer*)mPlayer)->prepare();
	}
	
	int TinaPlayer::start()
	{
		#if SAVE_PCM_DATA
			savaPcmFd = fopen("/mnt/UDISK/save.pcm", "wb");
			if(savaPcmFd==NULL){
				loge("fopen save.pcm fail****");
				loge("err str: %s",strerror(errno));
			}else{
				fseek(savaPcmFd,0,SEEK_SET);
			}
		#endif
		logd("tinaplayer start begin");
		int ret = ((AwPlayer*)mPlayer)->start();
		logd("tinaplayer start finish");
		return ret;
	}
	
	int TinaPlayer::stop()
	{
		#if SAVE_PCM_DATA
			if(savaPcmFd!=NULL){
				fclose(savaPcmFd);
				savaPcmFd = NULL;
			}
		#endif
		return ((AwPlayer*)mPlayer)->stop();
	}
	
	int TinaPlayer::pause()
	{
		return ((AwPlayer*)mPlayer)->pause();
	}
	
	int TinaPlayer::seekTo(int msec)
	{
		return ((AwPlayer*)mPlayer)->seekTo(msec);
	}
	
	int TinaPlayer::reset()
	{
		#if SAVE_PCM_DATA
			if(savaPcmFd!=NULL){
				fclose(savaPcmFd);
				savaPcmFd = NULL;
			}
		#endif
		pid_t tid = syscall(SYS_gettid);
		logd("TinaPlayer::reset() begin,tid = %d",tid);
		struct timeval time1, time2, time3;
		memset(&time3, 0, sizeof(struct timeval));
		gettimeofday(&time1, NULL);
		int ret = ((AwPlayer*)mPlayer)->reset();
		gettimeofday(&time2, NULL);
		time3.tv_sec += (time2.tv_sec-time1.tv_sec);
		time3.tv_usec += (time2.tv_usec-time1.tv_usec);
		logd("TinaPlayer::reset() >>> time elapsed: %ld seconds  %ld useconds\n", time3.tv_sec, time3.tv_usec);
		logd("TinaPlayer::reset() finish,tid = %d",tid);
		return ret;
	}
	
	
	int TinaPlayer::isPlaying()
	{
		return ((AwPlayer*)mPlayer)->isPlaying();
	}
	
	
	int TinaPlayer::getCurrentPosition(int* msec)
	{
		return ((AwPlayer*)mPlayer)->getCurrentPosition(msec);
	}
	
	
	int TinaPlayer::getDuration(int *msec)
	{
		return ((AwPlayer*)mPlayer)->getDuration(msec);
	}
	
	
	int TinaPlayer::setLooping(int loop)
	{
		mLoop = loop;
		return ((AwPlayer*)mPlayer)->setLooping(loop);
	}

	/*now,this function do not do */
	int TinaPlayer::setVolume(int volume)
	{
		return ((AwPlayer*)mPlayer)->setVolume(volume);
	}

	void TinaPlayer::callbackToApp(int msg, int param0, void* param1){
		if(mNotifier){
			mNotifier(mUserData,msg,param0,param1);
		}else{
			loge(" mNotifier is null ");
		}
	}

}

