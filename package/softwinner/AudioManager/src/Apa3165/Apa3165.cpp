#include "Apa3165.h"

namespace softwinner {

Apa3165::Apa3165(){
    mVolume = 30;
    mMaxVolume = mixer_getmaxvolume(VolumeName);
    setVolume(mVolume);
}

void Apa3165::setVolume(int value){
    if( value > 100 )
        mVolume = 100; 
    else mVolume = value;

    mixer_set(VolumeName,(100 - mVolume)*mMaxVolume/100);
}
int Apa3165::getVolume(){
    return mVolume;
}
int Apa3165::getMaxVolumeHw(){
    return mMaxVolume;
}

}/*namespace softwinner*/