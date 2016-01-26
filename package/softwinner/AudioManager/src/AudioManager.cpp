#define TAG "AudioManager"
#include <tina_log.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <dlfcn.h>
#include <sys/epoll.h>

#include "AudioManager.h"

#define PRESS_DATA_NAME    "sndi2s0 sunxi linein Jack" 
//#define PRESS_DATA_NAME    "gpio-keys-polled" 
//#define PRESS_DATA_NAME    "axp22-supplyer" 
#define PRESS_SYSFS_PATH   "/sys/class/input"
/*-------------------------------------------------------------*/

int linein_get_class_path(char *class_path)
{
    char dirname[] = PRESS_SYSFS_PATH;
    char buf[256];
    int res;
    DIR *dir;
    struct dirent *de;
    int fd = -1;
    int found = 0;

    dir = opendir(dirname);
    if (dir == NULL)
        return -1;

    while((de = readdir(dir))) {
        if (strncmp(de->d_name, "input", strlen("input")) != 0) {
            continue;
        }

        sprintf(class_path, "%s/%s", dirname, de->d_name);
        snprintf(buf, sizeof(buf), "%s/name", class_path);

        fd = open(buf, O_RDONLY);
        if (fd < 0) {
            continue;
        }
        if ((res = read(fd, buf, sizeof(buf))) < 0) {
            close(fd);
            continue;
        }
        buf[res - 1] = '\0';
        if (strcmp(buf, PRESS_DATA_NAME) == 0) {
            found = 1;
            close(fd);
            break;
        }

        close(fd);
        fd = -1;
    }
    closedir(dir);
    if (found) {
        return 0;
    }else {
        *class_path = '\0';
        return -1;
    }
}

/*-------------------------------------------------------------*/
namespace softwinner {
AudioManager::AudioManager(){
    mStop = true;
    mEventfd = -1;
    mEpollfd = -1;
    mAudioAuxInStatusListener = NULL;
}
AudioManager::~AudioManager(){
    if(mStop == false){
        destoryAudioAuxInDetect();
    }
}

int AudioManager::mixer_get(const char* shell,const char* name){
    int bytes;
    char buf[10];

    char cmd[500];
    sprintf(cmd,shell,name);
    FILE   *stream;
    TLOGD("%s\n",cmd);
    stream = popen( cmd, "r" );
    if(!stream) return -1;
    bytes = fread( buf, sizeof(char), sizeof(buf), stream);
    pclose(stream);
    if(bytes > 0){
            return atoi(buf);
    }else {
            TLOGE("%s --> failed\n",cmd);
            return -1;
    }
}
int AudioManager::mixer_getcurvolume(const char* name){
    const char* shell = "volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
    return mixer_get(shell,name);
}

int AudioManager::mixer_getmaxvolume(const char* name){
    const char* shell = "volume=`amixer cget name='%s' | grep max `; volume=${volume#*max=}; volume=${volume%%,*};echo $volume";
    return mixer_get(shell,name);
}

void AudioManager::mixer_set(const char* name, int value){
    char cmd[100];
    sprintf(cmd,"amixer cset name='%s' %d",name,value);
    system(cmd); 
}

int AudioManager::initAudioAuxInDetect(AudioAuxInStatusListener* l){
    
    mAudioAuxInStatusListener = l;

    int i = 0;
    int len = 0;
    char class_path[100];

    memset(class_path,0,sizeof(class_path));
    linein_get_class_path(class_path);
    len = strlen(class_path);
    sprintf(class_path, "/dev/input/event%c", class_path[len - 1]);
    TLOGD("path: %s\n",class_path);

    mEventfd = open(class_path, O_RDONLY); //may be the powerlinein is /dev/input/event1
    if (mEventfd < 0) { 
        TLOGE("can not open device usblineinboard!(%s)",strerror(errno)); 
        return -1; 
    } 

    //start thread
    mStop = false;
    run();

    return 0;
}

void AudioManager::destoryAudioAuxInDetect(){
    mStop = true;
    int ret = close(mEventfd);
}

int AudioManager::loop(){
    struct input_event buff;
    
    read(mEventfd,&buff,sizeof(struct input_event));
    
    //FIXME: bug here form destoryAudioAuxInDetect
    //When the destoryAudioAuxInDetect coming, 
    //the thread still block in read, (different from socket fd)
    //So, the thread exit next event comming
    //use epoll or poll ??
    if(mStop){
        TLOGI("AudioAuxInDetect thread exit");
        return Thread::THREAD_EXIT;
    }

    TLOGD("%s,l:%d,buff.code:%d,buff.value:%d\n", __FUNCTION__, __LINE__, buff.code,buff.value);
    if(buff.code != 0 && buff.value == AudioAuxInStatusListener::STATUS_IN && mAudioAuxInStatusListener != NULL) {
        mAudioAuxInStatusListener->onAudioInStatusChannged(AudioAuxInStatusListener::STATUS_IN,buff.time);
    } else if (buff.code != 0 && buff.value == AudioAuxInStatusListener::STATUS_SIGNAL && mAudioAuxInStatusListener != NULL) {
        mAudioAuxInStatusListener->onAudioInStatusChannged(AudioAuxInStatusListener::STATUS_SIGNAL,buff.time);
    } else if (buff.code != 0 && buff.value == AudioAuxInStatusListener::STATUS_OUT && mAudioAuxInStatusListener != NULL) {
        mAudioAuxInStatusListener->onAudioInStatusChannged(AudioAuxInStatusListener::STATUS_OUT,buff.time);
    }
    return Thread::THREAD_CONTINUE;
}
}
