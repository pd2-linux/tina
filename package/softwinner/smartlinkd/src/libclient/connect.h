#ifndef _CONNECT_H_
#define _CONNECT_H_

#include "smartlink_util.h"

#ifdef __cplusplus
extern "C" {
#endif
const int THREAD_INIT = -100;
const int THREAD_EXIT = 1;
const int THREAD_CONTINUE = 0;

/*********** app client API ***********/
void aw_smartlinkd_prepare();
int aw_smartlinkd_init(int fd,int(f)(char*,int));

int aw_startairkiss();
int aw_startcooee();

int aw_stopairkiss();
int aw_stopcooee();

/*********** no use for app client ***********/
int aw_easysetupfinish(struct _cmd *c);

/***************** for debug *****************/
void printf_info(struct _cmd *c);

#ifdef __cplusplus
}
#endif
#endif /* _CONNECT_H_ */
