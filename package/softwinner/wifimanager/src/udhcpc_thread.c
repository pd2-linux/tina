#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "wifi_event.h"

extern int disconnecting;
static tWifi_event_callback p_state_callback = NULL;

static int get_net_ip(const char *if_name, char *ip, int *len, int *vflag)
{
    struct ifaddrs * ifAddrStruct=NULL;
    void * tmpAddrPtr=NULL;

    *vflag = 0;
    getifaddrs(&ifAddrStruct);
    while (ifAddrStruct!=NULL) {
       if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4                                                 is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            if(strcmp(ifAddrStruct->ifa_name, if_name) == 0){
                inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
                *vflag = 4;
                break;
            }
       } else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            if(strcmp(ifAddrStruct->ifa_name, if_name) == 0){
                inet_ntop(AF_INET6, tmpAddrPtr, ip, INET6_ADDRSTRLEN);    
                *vflag=6;
                break;
            }
       } 
       ifAddrStruct=ifAddrStruct->ifa_next;
    }
    
    if(ifAddrStruct != NULL){
        freeifaddrs(ifAddrStruct);
    }
    return 0;    	
}

int is_ip_exist()
{
    int len = 0, vflag = 0;
    char ipaddr[INET6_ADDRSTRLEN];
    
    get_net_ip("wlan0", ipaddr, &len, &vflag);
    return vflag;	
}

void *udhcpc_thread(void *args)
{
	  int len = 0, vflag = 0, times = 0;
	  char ipaddr[INET6_ADDRSTRLEN];
	  
    /* restart udhcpc */
	  system("/etc/wifi/udhcpc_wlan0 restart");
	  
	  /* check ip exist */
	  len = INET6_ADDRSTRLEN;
	  times = 0;
	  do{
	      usleep(100000);
	      if(disconnecting == 1){
	          printf("receive disconnect cmd!\n");
	          system("/etc/wifi/udhcpc_wlan0 stop");
	          break;
	      }
	      get_net_ip("wlan0", ipaddr, &len, &vflag);
	      times++;    
	  }while((vflag == 0) && (times < 120));
	  
	  if(p_state_callback != NULL){
	      if(vflag != 0){
	          p_state_callback(NETWORK_CONNECTED, args);
	      }else{
	      	  printf("udhcpc wlan0 timeout!\n");
	          p_state_callback(CONNECT_TIMEOUT, NULL);
	      }    
	  }
	  
	  pthread_exit(NULL);
}


void start_udhcpc_thread(tWifi_event_callback pcb, void *args)
{
    pthread_t thread_id;
    p_state_callback = pcb;
    pthread_create(&thread_id, NULL, &udhcpc_thread, args);   
}