#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__
#include <stdlib.h>
#include <stdio.h>

#include <thread.h>

namespace softwinner {

class AudioManager : public Thread {
public:
    AudioManager();
    virtual ~AudioManager();

    /*relative volume 1~100(%)*/
    virtual void setVolume(int value) = 0;
    virtual int getVolume() = 0;
    virtual int setChannel(int channel) = 0;

    class AudioAuxInStatusListener{
    public:
        enum Status
        {
            STATUS_OUT = 128,
            STATUS_IN = 32,
            STATUS_SIGNAL = 64,
        };
        virtual void onAudioInStatusChannged(enum Status status, struct timeval time) = 0;
    };

    int initAudioAuxInDetect(AudioAuxInStatusListener* l);
    void destoryAudioAuxInDetect();

protected:
    /*hardware absolute volume*/
    virtual int getMaxVolumeHw() = 0;

    int mixer_get(const char* shell,const char* name);
    int mixer_getcurvolume(const char* name);
    int mixer_getmaxvolume(const char* name);
    void mixer_set(const char* name, int value);

    int mVolume;
    int mMaxVolume;

private:
    AudioAuxInStatusListener* mAudioAuxInStatusListener;
    int mEventfd;
    int mEpollfd;

    int loop();
};
}/*namespace softwinner*/
#endif /*__AUDIO_MANAGER_H__*/
