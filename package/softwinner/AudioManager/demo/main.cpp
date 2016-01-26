#include "AudioManager.h"
#include "Apa3165.h"
#include <unistd.h>
#include <tina_log.h>

using namespace softwinner;
class AudioAuxInStatusListener : public AudioManager::AudioAuxInStatusListener
{
    void onAudioInStatusChannged(enum Status event, struct timeval time){
        TLOGD("onAudioInStatusChannged: %d",event);
    }
};

int main(){
    AudioManager* am = new Apa3165();
    AudioAuxInStatusListener al;   
    if( am->initAudioAuxInDetect(&al) == -1){
        return -1;
    }

    sleep(1000);
    am->destoryAudioAuxInDetect();
    while(1);
}
