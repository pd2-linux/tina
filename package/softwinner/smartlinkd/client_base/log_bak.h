#include <stdio.h>

/* direct log message to your system utils, default printf */
extern int debug_enable;
#define LOGE(fmt, arg...) do {if (1 | debug_enable) printf(fmt, ##arg);} while (0);
#define LOGD(fmt, arg...) do {if (debug_enable) printf(fmt, ##arg);} while (0);