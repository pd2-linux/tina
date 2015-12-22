#ifndef __TINA_LOG_H__
#define __TINA_LOG_H__

#include <syslog.h>


#define OPTION_LOG_LEVEL_CLOSE      0
#define OPTION_LOG_LEVEL_DEBUG      1
#define OPTION_LOG_LEVEL_WARNING    2
#define OPTION_LOG_LEVEL_DEFAULT    3
#define OPTION_LOG_LEVEL_DETAIL     4

#define LOG_LEVEL_ERROR     "E/"
#define LOG_LEVEL_WARNING   "W/"
#define LOG_LEVEL_INFO      "I/"
#define LOG_LEVEL_VERBOSE   "V/"
#define LOG_LEVEL_DEBUG     "D/"

#ifndef TAG
#define TAG "Tina-default"
#endif

#ifndef LOG_TAG
#define LOG_TAG "["TAG"]"
#endif

#ifndef CONFIG_LOG_LEVEL
#define CONFIG_LOG_LEVEL OPTION_LOG_LEVEL_DEFAULT
#endif

#define TLOG(systag, level, fmt, arg...)  \
        syslog(systag, "%s:%s <%s:%u>: "fmt"\n", level, LOG_TAG, __FUNCTION__, __LINE__, ##arg)


#define TLOGE(fmt, arg...) TLOG(LOG_ERR,LOG_LEVEL_ERROR, fmt, ##arg)

#if CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_CLOSE

#define TLOGD(fmt, arg...)
#define TLOGW(fmt, arg...)
#define TLOGI(fmt, arg...)
#define TLOGV(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_DEBUG

#define TLOGD(fmt, arg...) TLOG(LOG_DEBUG,LOG_LEVEL_DEBUG, fmt, ##arg)
#define TLOGW(fmt, arg...)
#define TLOGI(fmt, arg...)
#define TLOGV(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_WARNING

#define TLOGD(fmt, arg...) TLOG(LOG_DEBUG,LOG_LEVEL_DEBUG, fmt, ##arg)
#define TLOGW(fmt, arg...) TLOG(LOG_WARNING,LOG_LEVEL_WARNING, fmt, ##arg)
#define TLOGI(fmt, arg...)
#define TLOGV(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_DEFAULT

#define TLOGD(fmt, arg...) TLOG(LOG_DEBUG,LOG_LEVEL_DEBUG, fmt, ##arg)
#define TLOGW(fmt, arg...) TLOG(LOG_WARNING,LOG_LEVEL_WARNING, fmt, ##arg)
#define TLOGI(fmt, arg...) TLOG(LOG_INFO,LOG_LEVEL_INFO, fmt, ##arg)
#define TLOGV(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_DETAIL

#define TLOGD(fmt, arg...) TLOG(LOG_DEBUG,LOG_LEVEL_DEBUG, fmt, ##arg)
#define TLOGW(fmt, arg...) TLOG(LOG_WARNING,LOG_LEVEL_WARNING, fmt, ##arg)
#define TLOGI(fmt, arg...) TLOG(LOG_INFO,LOG_LEVEL_INFO, fmt, ##arg)
#define TLOGV(fmt, arg...) TLOG(LOG_INFO,LOG_LEVEL_VERBOSE, fmt, ##arg)

#endif

#endif /*__TINA_LOG_H__*/


