#ifndef _CONNECT_H_
#define _CONNECT_H_

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif
#define THREAD_INIT -100
#define THREAD_EXIT 1
#define THREAD_CONTINUE 0
int init(int fd,int(f)(char*,int));
void release();

int easysetupfinish(struct _cmd *c);

int startairkiss();

void printf_info(struct _cmd *c);

#ifdef __cplusplus
}
#endif
#endif /* _CONNECT_H_ */
