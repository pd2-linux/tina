#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include "wifi.h"
#include "wifi_event.h"
#include "wifi_state_machine.h"
#include "wifi_intf.h"
#include "wpa_supplicant_conf.h"

tWIFI_STATE  gwifi_state;
extern char netid_connecting[];
extern int  connecting_ap_event_label;
extern int  disconnect_ap_event_label;
extern void start_udhcpc_thread(void *args);

tWifi_event_callback wifi_event_callback[MAX_CALLBCAKS_COUNT] = {NULL};
int wifi_event_callback_index = 0;

static pthread_t event_thread_id;
static int wifi_event_inner = AP_DISCONNECTED;
static int scan_complete = 0;
static int assoc_reject_count = 0;
static int authentication_fail_count = 0;

static void handle_event(int event, char * remainder) {
    char netid_connected[NET_ID_LEN+1] = {0};
    char cmd[255] = {0}, reply[16]={0};
    int len = NET_ID_LEN + 1;
    tWIFI_MACHINE_STATE state;       
            
    switch (event){
        case DISCONNECTED:
			state = get_wifi_machine_state();
			if((state == DISCONNECTING_STATE) //call disconnect
				|| (state == L2CONNECTED_STATE) || (state == CONNECTED_STATE)) //auto disconnect(ap shutdown)
			{
				printf("Network disconnected!\n");
				set_wifi_machine_state(DISCONNECTED_STATE);
				set_cur_wifi_event(AP_DISCONNECTED);
				call_event_callback_function(WIFIMG_NETWORK_DISCONNECTED, NULL, disconnect_ap_event_label);
			}
			break;
        
        case CONNECTED:
            if(netid_connecting[0] != '\0'){
                /* get already connected netid */
                wpa_conf_get_ap_connected(netid_connected, &len);
                printf("connecting id %s, connected id %s\n", netid_connecting, netid_connected);
                if(strcmp(netid_connected,netid_connecting) != 0){
                    /* send disconnect */			
                    sprintf(cmd, "%s", "DISCONNECT");
                    wifi_command(cmd, reply, sizeof(reply));
                    break;
                }
            }
            
            assoc_reject_count = 0;
            
            set_wifi_machine_state(L2CONNECTED_STATE);
            /* start udhcpcd */
            start_udhcpc_thread((void *)remainder);
            break;
        
        case SCAN_RESULTS:
            scan_complete = 1;
            break;
        
        case UNKNOWN:
            //printf("unknown event!\n");
            break;
    }
}

static int dispatch_event(const char *event_str, int nread)
{
	  int i = 0, event = 0;
    char event_name[16];
    char cmd[255] = {0}, reply[16]={0};
	  char *name_start = NULL, *name_end = NULL;
	  char *event_data = NULL;
	  
    if(!event_str || !event_str[0]){
        printf("event is NULL!\n");
        return 0;
    }
    
    if(strncmp(event_str, "CTRL-EVENT-", 11)){
        if (!strncmp(event_str, "WPA:", 4)){
            if (strstr(event_str, "pre-shared key may be incorrect")){
                authentication_fail_count++;
				
				printf("pre-shared key incorrect!\n");
				printf("authentication_fail_count %d\n", authentication_fail_count);
				
                if(authentication_fail_count >= MAX_RETRIES_ON_AUTHENTICATION_FAILURE){
                    /* send disconnect */			
                    sprintf(cmd, "%s", "DISCONNECT");
                    wifi_command(cmd, reply, sizeof(reply));
                
                    set_wifi_machine_state(DISCONNECTED_STATE);
					set_cur_wifi_event(PASSWORD_INCORRECT);
                }
                return 0;
            }
        }
        
        //printf("EVENT, not care!\n");
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
        gwifi_state = WIFIMG_WIFI_DISABLED;
        call_event_callback_function(WIFIMG_WIFI_OFF_SUCCESS, NULL, 0);
        return 1;
    }else if(event == EAP_FAILURE){
        printf("EAP FAILURE!\n");
        return 0;
    }else if(event == ASSOC_REJECT){
        assoc_reject_count++;
        if(assoc_reject_count >= MAX_ASSOC_REJECT_COUNT){
            /* send disconnect */			
            sprintf(cmd, "%s", "DISCONNECT");
            wifi_command(cmd, reply, sizeof(reply));
        }   	
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
    
    pthread_exit(NULL);	
}

void wifi_event_loop(tWifi_event_callback pcb)
{
    /* initial */
    wifi_event_inner = AP_DISCONNECTED;   
    pthread_create(&event_thread_id, NULL, &event_handle_thread, NULL);
}

tWIFI_EVENT_INNER  get_cur_wifi_event()
{
    return 	wifi_event_inner;
}

int set_cur_wifi_event(tWIFI_EVENT_INNER event)
{
	wifi_event_inner = event;
	return 0;
}

void *check_connect_timeout(void *args)
{
	int i = 0;
    tWIFI_MACHINE_STATE  state;
    tWIFI_EVENT_INNER    event;
    char cmd[256] = {0}, reply[8] = {0};
    
    i = 0;
    do {
        usleep(100000);

        state = get_wifi_machine_state();
        event = get_cur_wifi_event();

        /* password incorrect */
        if ((state == DISCONNECTED_STATE) && (event == PASSWORD_INCORRECT)){
			printf("check_connect_timeout exit: password failed!\n");
			call_event_callback_function(WIFIMG_PASSWORD_FAILED, NULL, connecting_ap_event_label);
			break;
        }

        if(assoc_reject_count >= MAX_ASSOC_REJECT_COUNT){ 
			printf("associat reject over 3 times!\n");
            assoc_reject_count = 0;
            break;
        }

        i++;
    } while((state != L2CONNECTED_STATE) && (state != CONNECTED_STATE) && (i < 200));
		
    /* wifi not exist or can't connect */
	if (get_wifi_machine_state() == CONNECTING_STATE){
        /* send disconnect */			
        sprintf(cmd, "%s", "DISCONNECT");
        wifi_command(cmd, reply, sizeof(reply));

		set_wifi_machine_state(DISCONNECTED_STATE);
		set_cur_wifi_event(CONNECT_AP_TIMEOUT);
		call_event_callback_function(WIFIMG_CONNECT_TIMEOUT, NULL, connecting_ap_event_label);
	}
		
	pthread_exit(NULL);
}


void start_check_connect_timeout(int first)
{
    pthread_t check_timeout_id;
    pthread_create(&check_timeout_id, NULL, &check_connect_timeout, NULL);
}

void *wifi_on_check_connect_timeout(void *args)
{
	  int i = 0;
    char cmd[255] = {0}, reply[16]={0};
    tWIFI_MACHINE_STATE  state;
    tWIFI_EVENT_INNER    event;
    	
    /* sync wpa_supplicant state */
    i = 0;
    do {
        usleep(100000);

        state = get_wifi_machine_state();
        event = get_cur_wifi_event();

		i++;
    } while((state != L2CONNECTED_STATE) && (state != CONNECTED_STATE) && (i < 100));
    
    if((state != L2CONNECTED_STATE) && (state != CONNECTED_STATE)){
    	  /* send disconnect */
        sprintf(cmd, "%s", "DISCONNECT");
        wifi_command(cmd, reply, sizeof(reply));

        set_wifi_machine_state(DISCONNECTED_STATE);
        printf("call event NO_NETWORK_CONNECTING\n");
        call_event_callback_function(WIFIMG_NO_NETWORK_CONNECTING, NULL, connecting_ap_event_label);
    }
    
    pthread_exit(NULL);
}

void start_wifi_on_check_connect_timeout()
{
    pthread_t wifi_on_check_timeout_id;
    pthread_create(&wifi_on_check_timeout_id, NULL, &wifi_on_check_connect_timeout, NULL);	
}


void set_scan_start_flag()
{
    scan_complete = 0;	
}

int get_scan_status()
{
    return scan_complete;
}

void reset_assoc_reject_count()
{
	assoc_reject_count = 0;
}

int get_assoc_reject_count()
{
	return assoc_reject_count;
}

int add_wifi_event_callback_inner(tWifi_event_callback pcb)
{
    if(wifi_event_callback_index >= MAX_CALLBCAKS_COUNT){
        return -1;
    }
    
    wifi_event_callback[wifi_event_callback_index]=pcb;
    wifi_event_callback_index++;
    return 0;
}

int reset_wifi_event_callback()
{
    int i=0;
    for(i=0; i<wifi_event_callback_index; i++){
        wifi_event_callback[i]=NULL;
    }
    return 0;
}

int call_event_callback_function(tWIFI_EVENT wifi_event, char *buf, int event_label)
{
    int i=0;
    
    printf("call event 0x%x\n", wifi_event);

    for(i=0; i<wifi_event_callback_index; i++){
        if(wifi_event_callback[i] != NULL){
            wifi_event_callback[i](wifi_event, buf, event_label);
        }
    }
    
    return 0;    	
}
