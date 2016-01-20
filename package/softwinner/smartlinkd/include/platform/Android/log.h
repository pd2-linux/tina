#ifndef __ANDOIRD_LOG_H__
#define __ANDOIRD_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _JNI_
#define LOG_TAG "smartlinkd_jni"
#else
#define LOG_TAG "smartlinkd"
#endif

//#include <android/log.h>
#include <cutils/log.h>

#define DBG_EN 1

#if DBG_EN
#define LOGD(x,arg...) ALOGD("[debug] "x,##arg)
//#define LOGD printf
#else
#define LOGD(x,arg...) ALOGD
#endif

#define LOGE(x,arg...) ALOGE("[error] "x,##arg)
//#define LOGE printf

#ifdef __cplusplus
}
#endif

#endif
