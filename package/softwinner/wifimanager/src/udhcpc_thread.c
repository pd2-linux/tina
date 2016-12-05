#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "wifi_event.h"
#include "wifi_state_machine.h"
#include "wifi.h"

extern int disconnecting;
extern int  connecting_ap_event_label;
extern int  disconnect_ap_event_label;

static int get_net_ip(const char *if_name, char *ip, int *len, int *vflag)
{
    struct ifaddrs * ifAddrStruct=NULL, *pifaddr=NULL;
    void * tmpAddrPtr=NULL;

    *vflag = 0;
    getifaddrs(&ifAddrStruct);
    pifaddr = ifAddrStruct;
    while (pifaddr!=NULL) {
       if (pifaddr->ifa_addr->sa_family==AF_INET) { // check it is IP4
            tmpAddrPtr=&((struct sockaddr_in *)pifaddr->ifa_addr)->sin_addr;
            if(strcmp(pifaddr->ifa_name, if_name) == 0){
                inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
                *vflag = 4;
                break;
            }
       } else if (pifaddr->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in *)pifaddr->ifa_addr)->sin_addr;
            if(strcmp(pifaddr->ifa_name, if_name) == 0){
                inet_ntop(AF_INET6, tmpAddrPtr, ip, INET6_ADDRSTRLEN);    
                *vflag=6;
                break;
            }
       } 
       pifaddr=pifaddr->ifa_next;
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
    char cmd[256] = {0}, reply[8] = {0};
	  
    /* restart udhcpc */
    system("/etc/wifi/udhcpc_wlan0 restart");
	  
    /* check ip exist */
    len = INET6_ADDRSTRLEN;
    times = 0;
    do{
        usleep(100000);
        if(disconnecting == 1){
            system("/etc/wifi/udhcpc_wlan0 stop");
            break;
        }
        get_net_ip("wlan0", ipaddr, &len, &vflag);
        times++;
    }while((vflag == 0) && (times < 310));
	  
    printf("vflag= %d\n",vflag);
    if(vflag != 0){
        set_wifi_machine_state(CONNECTED_STATE);
		set_cur_wifi_event(AP_CONNECTED);
        call_event_callback_function(WIFIMG_NETWORK_CONNECTED, NULL, connecting_ap_event_label);
    }else{
        printf("udhcpc wlan0 timeout, pid %d!\n",pthread_self());
        /* stop dhcpc thread */
        system("/etc/wifi/udhcpc_wlan0 stop");
    	  
    	/* send disconnect */			
        sprintf(cmd, "%s", "DISCONNECT");
        wifi_command(cmd, reply, sizeof(reply));
        
        set_wifi_machine_state(DISCONNECTED_STATE);
		set_cur_wifi_event(OBTAINING_IP_TIMEOUT);
        call_event_callback_function(WIFIMG_CONNECT_TIMEOUT, NULL, connecting_ap_event_label);
    }
	  
    pthread_exit(NULL);
}


void start_udhcpc_thread(void *args)
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, &udhcpc_thread, args);
}
