#define TAG "smartlinkd-demo"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "connect.h"
#include "log.h"
int debug_enable = 1;

int onRead(char* buf,int length)
{
	if(length == THREAD_INIT){
		
	}
	else if(length == -1){
		
	}else if(length == 0){
		LOGD("server close the connection...\n");
	}else {
		LOGD("lenght: %d\n",length);
		printf_info((struct _cmd *)buf);
	}
	return THREAD_EXIT;
}

int main(int argc, char* argv[])  
{  
	int proto = 0;
	if(argc > 1){
		proto = atoi(argv[1]);
	}

	prepare();
	if(init(0,onRead) == 0){
		if(proto == 0){
			LOGD("start airkiss\n");
			startairkiss();
		}
		else if(proto == 1){
			LOGD("start cooee\n");
			startcooee();
		}
	}
	while(1);

    return 0;  
}
