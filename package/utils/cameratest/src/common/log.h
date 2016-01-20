#ifndef __LOG_H__
#define __LOG_H__

#define log_dbg(x,arg...) printf("[debug] "x,##arg)
#define log_err(x,arg...) printf("[error] "x,##arg)

#define DBG_EN 1
/*
#define LOG_TAG "bell-socket-network"
#include <cutils/log.h>
#include <android/log.h>
#undef log_dbg
#undef log_err
#if DBG_EN
#define log_dbg(x,arg...) ALOGD("[debug] "x,##arg)
#else
#define log_dbg(x,arg...) ALOGD
#endif

#define log_err(x,arg...) ALOGE("[error] "x,##arg)
*/
#endif
