#ifndef __APA3165_H__
#define __APA3165_H__
#include "AudioManager.h"
namespace softwinner {

class Apa3165 : public AudioManager 
{
public:
    Apa3165();
    virtual ~Apa3165(){};

    void setVolume(int value);
    int getVolume();
    int setChannel(int channel){return 0;};

    const char* VolumeName = "Master Playback Volume";
protected:
    int getMaxVolumeHw();    
};
}/*namespace softwinner*/
#endif /*__APA3165_H__*/