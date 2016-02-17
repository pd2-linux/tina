#include <pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "wifi.h"
#include "wifi_event.h"
#include "wifi_intf.h"

extern int gwifi_state;
static tWifi_event_callback p_event_callback = NULL;

static void handle_event(int event, char * remainder) {
    switch (event){
        case DISCONNECTED:
        	  printf("Network disconnected!\n");
        	  gwifi_state = NETWORK_DISCONNECTED;
        	  if (p_event_callback) {
        	      p_event_callback(NETWORK_DISCONNECTED, remainder);
        	  }
        	  break;
        
        case CONNECTED:
            printf("Network connected!\n");
            gwifi_state = NETWORK_CONNECTED;
            printf("Connected event data: %s\n", remainder);
            if (p_event_callback) {
        	      p_event_callback(NETWORK_CONNECTED, remainder);
        	  }
            break;
        
        case SCAN_RESULTS:
            printf("scan results:\n");
            break;
        
        case UNKNOWN:
            printf("unknown event!\n");
            break;
    }
}

static int dispatch_event(const char *event_str, int nread)
{
	  int i = 0, event = 0;
    char event_name[16];
	  char *name_start = NULL, *name_end = NULL;
	  char *event_data = NULL;
	    
    if(!event_str || !event_str[0]){
        printf("event is NULL!\n");
        return 0;
    }
    
    if(strncmp(event_str, "CTRL-EVENT-", 11)){
        if (!strncmp(event_str, "WPA:", 4)){
            if (strstr(event_str, "pre-shared key may be incorrect")){
                gwifi_state = PASSWORD_FAILED;
                if (p_event_callback) {
                    p_event_callback(PASSWORD_FAILED, NULL);
                }
                return 0;
            }
        }
        
        printf("EVENT, not care!\n");
        return 0;
    }
    
    name_start = (char *)((unsigned int)event_str+11);
    name_end = strchr(name_start, ' ');
    if(name_end){
        while((name_start < name_end) && (i < 15)){
            event_name[i] = *name_start++;
            i++;    	
        }
        event_name[i] = '\0';
    } else {
        printf("Received wpa_supplicant event with empty event name!\n");
        return 0;
    }
    
    printf("event CTRL-EVENT-%s\n", event_name);
    
    
    /*
     * Map event name into event enum
    */
    if(!strcmp(event_name, "CONNECTED")){
        event = CONNECTED;
    }else if(!strcmp(event_name, "DISCONNECTED")){
        event = DISCONNECTED;
    }else if(!strcmp(event_name, "STATE-CHANGE")){
        event = STATE_CHANGE;
    }else if(!strcmp(event_name, "SCAN-RESULTS")){
        event = SCAN_RESULTS;
    }else if(!strcmp(event_name, "LINK-SPEED")){
        event = LINK_SPEED;
    }else if(!strcmp(event_name, "TERMINATING")){
        event = TERMINATING;
    }else if(!strcmp(event_name, "DRIVER-STATE")){
        event = DRIVER_STATE;
    }else if(!strcmp(event_name, "EAP-FAILURE")){
        event = EAP_FAILURE;
    }else if(!strcmp(event_name, "ASSOC-REJECT")){
        event = ASSOC_REJECT;
    }else{
        event = UNKNOWN;
    }

    event_data = (char *)((unsigned int)event_str);
    if(event == DRIVER_STATE || event == LINK_SPEED){
        printf("DRIVER_STATE or LINK_SPEED, not handle\n");
        return 0;
    }else if(event == STATE_CHANGE || event == EAP_FAILURE){
        event_data = strchr(event_str, ' ');
        if(event_data){
            event_data++;
        }
    }else{
        event_data = strstr(event_str, " - ");
        if (event_data) {
            event_data += 3;
        }
    }
    
    if(event == STATE_CHANGE){
        printf("STATE_CHANGE, no care!\n");
        return 0;
    } else if(event == DRIVER_STATE){
        printf("DRIVER_STATE, no care!\n");
        return 0;
    }else if(event == TERMINATING){
        printf("Wpa supplicant terminated!\n");
        return 1;
    }else if(event == EAP_FAILURE){
        printf("EAP FAILURE!\n");
        return 0;
    }else if(event == ASSOC_REJECT){
        printf("ASSOC REJECT!\n");
        return 0;
    }else{
        handle_event(event, event_data);
    }

    return 0;
}

void *event_handle_thread(void* args)
{
    int nread = 0, ret = 0;
    char buf[EVENT_BUF_SIZE] = {0};
	  
    for(;;){
        int nread = wifi_wait_for_event(buf, sizeof(buf));
        if (nread > 0) {
            ret = dispatch_event(buf, nread);
            if(ret == 1){
                break;  // wpa_supplicant terminated
            }
        } else {
            continue;
        }
    }
    
    printf("event handle thread exit!\n");	
}

void wifi_event_loop(tWifi_event_callback pcb)
{
    pthread_t thread_id;
    p_event_callback = pcb;
    pthread_create(&thread_id, NULL, &event_handle_thread, NULL);	
}