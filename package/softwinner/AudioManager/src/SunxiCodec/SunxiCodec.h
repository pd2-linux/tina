#ifndef __SUNXICODEC_H__
#define __SUNXICODEC_H__
#include "AudioManager.h"
namespace softwinner {

class SunxiCodec : public AudioManager 
{
public:
    SunxiCodec();
    virtual ~SunxiCodec(){};

    void setVolume(int value);
    int getVolume();
    int getMaxVolume();
    int setChannel(int channel){return 0;};

    const char* VolumeName = "earpiece volume control";
    const char* SpeakerFunction = "Speaker Function";
    
};
}/*namespace softwinner*/
#endif /*__SUNXICODEC_H__*/