#define TAG "wifi"
#include <tina_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <errno.h>
#include <unistd.h>
#include <wifi_intf.h>

static int event = WIFIMG_NETWORK_DISCONNECTED;

static void wifi_event_handle(tWIFI_EVENT wifi_event, void *buf, int event_label)
{
    printf("event_label 0x%x\n", event_label);
    	
    switch(wifi_event)
    {
        case WIFIMG_WIFI_ON_SUCCESS:
        {
            printf("WiFi on success!\n");
            event = WIFIMG_WIFI_ON_SUCCESS;
            break;
        }
        
        case WIFIMG_WIFI_ON_FAILED:
        {
            printf("WiFi on failed!\n");
            event = WIFIMG_WIFI_ON_FAILED;
            break;
        }
        
        case WIFIMG_WIFI_OFF_FAILED:
        {
            printf("wifi off failed!\n");
            event = WIFIMG_WIFI_OFF_FAILED;
            break;
        }
        
        case WIFIMG_WIFI_OFF_SUCCESS:
        {
            printf("wifi off success!\n");
            event = WIFIMG_WIFI_OFF_SUCCESS;
            break;
        }
        
        case WIFIMG_NETWORK_CONNECTED:
        {
            printf("WiFi connected ap!\n");
            event = WIFIMG_NETWORK_CONNECTED;
            break;	
        }
        
        case WIFIMG_NETWORK_DISCONNECTED:
        {
            printf("WiFi disconnected!\n");
            event = WIFIMG_NETWORK_DISCONNECTED;
            break;	
        }
        
        case WIFIMG_PASSWORD_FAILED:
        {
            printf("Password authentication failed!\n");
            event = WIFIMG_PASSWORD_FAILED;
            break;	
        }
        
        case WIFIMG_CONNECT_TIMEOUT:
        {
            printf("Connected timeout!\n");
            event = WIFIMG_CONNECT_TIMEOUT;
            break;	
        }
        
        case WIFIMG_NO_NETWORK_CONNECTING:
        {
        	  printf("It has no wifi auto connect when wifi on!\n");
        	  event = WIFIMG_NO_NETWORK_CONNECTING;
        	  break;
        }
        
        case WIFIMG_CMD_OR_PARAMS_ERROR:
        {
            printf("cmd or params error!\n");
            event = WIFIMG_CMD_OR_PARAMS_ERROR;
            break;
        }
        
        case WIFIMG_KEY_MGMT_NOT_SUPPORT:
        {
            printf("key mgmt is not supported!\n");
            event = WIFIMG_KEY_MGMT_NOT_SUPPORT;
            break;
        }
        
        case WIFIMG_OPT_NO_USE_EVENT:
        {
            printf("operation no use!\n");
            event = WIFIMG_OPT_NO_USE_EVENT;
            break;
        }
        
        case WIFIMG_NETWORK_NOT_EXIST:
        {
            printf("network not exist!\n");
            event = WIFIMG_NETWORK_NOT_EXIST;
            break;	
        }
        
        case WIFIMG_DEV_BUSING_EVENT:
        {
            printf("wifi device busing!\n");
            event = WIFIMG_DEV_BUSING_EVENT;
            break;
        }
        
        default:
        {
            printf("Other event, no care!\n");
        }
    }
}

/*
 *argc[1]   ap ssid
 *argc[2]   ap passwd
*/
int main(int argv, char *argc[]){
    int ret = 0, len = 0;
    int times = 0, event_label = 0;;
    char ssid[256] = {0}, scan_results[4096] = {0};
    int  wifi_state = WIFIMG_WIFI_DISABLED;
    const aw_wifi_interface_t *p_wifi_interface = NULL;
    
    
    event_label = rand();
    p_wifi_interface = aw_wifi_on(wifi_event_handle, event_label);
    if(p_wifi_interface == NULL){
    	  printf("wifi on failed event 0x%x\n", event);
        return -1;
    }

    printf("wait wifi on event!\n");
    while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
        printf("wifi state busing,waiting\n");
        usleep(2000000);
    }
    
    event_label++;
    p_wifi_interface->disconnect_ap(event_label);
    while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
        printf("wifi state busing,waiting\n");
        usleep(2000000);
    }
    
    event_label++;
    p_wifi_interface->connect_ap_auto(event_label);
    while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
        printf("wifi state busing,waiting\n");
        usleep(2000000);
    }
    
    event_label++;
    p_wifi_interface->disconnect_ap(event_label);
    while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
        printf("wifi state busing,waiting\n");
        usleep(2000000);
    }
    
    event_label++;
    p_wifi_interface->connect_ap(argc[1], argc[2], event_label);
    
/*    
    aw_wifi_off(p_wifi_interface);
     
    p_wifi_interface->start_scan();
    len = 4096;
    p_wifi_interface->get_scan_results(scan_results, &len);
    printf("received scan results:\n");
    printf("%s\n", scan_results);


    printf("connect ap %s\n", argc[1]);
    wifi_state = aw_wifi_get_wifi_state();
    if((wifi_state == WIFIMG_WIFI_DISCONNECTED)
        || (wifi_state == WIFIMG_WIFI_CONNECTED)){
        event_label++;
        ret = p_wifi_interface->connect_ap(argc[1], argc[2], event_label);
        if(ret == WIFI_MANAGER_SUCCESS){
            printf("Call connect_ap OK!\n");
        
            printf("wait event!\n");
            times = 0;
            do{
                usleep(200000);
                times++;
                if((event == WIFIMG_NETWORK_CONNECTED)
                	  || (event == WIFIMG_PASSWORD_FAILED)
                	  || (event == WIFIMG_CONNECT_TIMEOUT)){
                	  break;
                }
            }while(1);
            
            wifi_state = aw_wifi_get_wifi_state();
            if(wifi_state == WIFIMG_WIFI_CONNECTED){
            	  len = 256;
                ret = p_wifi_interface->is_ap_connected(ssid, &len);
                if(ret){
                    printf("Connected ap %s\n", ssid);
                }
                
                /* 
                ret = p_wifi_interface->disconnect_ap(event_label);
                if(ret < 0)
                {
                    printf("call error! check event\n");
                }else{
                    printf("disconnect ap ok!");
                } 
                */
/*   
            }         
        }else{
            printf("call error! check event\n");
        }
    }else{
            	
        printf("WiFi isn't ready, waiting\n");
    }
*/

/*
    printf("connect ap xxxxxxx\n");
    wifi_state = aw_wifi_get_wifi_state();
    if((wifi_state == WIFIMG_WIFI_DISCONNECTED)
        || (wifi_state == WIFIMG_WIFI_CONNECTED)){
        event_label++;
        ret = p_wifi_interface->connect_ap("xxxxxxx", "1qaz@WSX", event_label);
        if(ret == WIFI_MANAGER_SUCCESS){
            printf("Call connect_ap OK!\n");
        
            printf("wait event!\n");
            times = 0;
            do{
                usleep(200000);
                times++;
                if((event == WIFIMG_NETWORK_CONNECTED)
                	  || (event == WIFIMG_PASSWORD_FAILED)
                	  || (event == WIFIMG_CONNECT_TIMEOUT)){
                	  break;
                }
            }while(1);
            
            wifi_state = aw_wifi_get_wifi_state();
            if(wifi_state == WIFIMG_WIFI_CONNECTED){
            	  len = 256;
                ret = p_wifi_interface->is_ap_connected(ssid, &len);
                if(ret){
                    printf("Connected ap %s\n", ssid);
                }
                
                ret = p_wifi_interface->disconnect_ap(event_label);
                if(ret < 0)
                {
                    printf("call error! check event\n");
                }else{
                    printf("disconnect ap ok!");
                }    
            }         
        }else{
            printf("call error! check event\n");
        }
    }else{
            	
        printf("WiFi isn't ready, waiting\n");
    }
    
    printf("connect ap AW2\n");
    wifi_state = aw_wifi_get_wifi_state();
    if((wifi_state == WIFIMG_WIFI_DISCONNECTED)
        || (wifi_state == WIFIMG_WIFI_CONNECTED)){
        event_label++;
        ret = p_wifi_interface->connect_ap("AW2", "1qaz@WSX", event_label);
        if(ret == WIFI_MANAGER_SUCCESS){
            printf("Call connect_ap OK!\n");
        
            printf("wait event!\n");
            times = 0;
            do{
                usleep(200000);
                times++;
                if((event == WIFIMG_NETWORK_CONNECTED)
                	  || (event == WIFIMG_PASSWORD_FAILED)
                	  || (event == WIFIMG_CONNECT_TIMEOUT)){
                	  break;
                }
            }while(1);
            
            wifi_state = aw_wifi_get_wifi_state();
            if(wifi_state == WIFIMG_WIFI_CONNECTED){
            	  len = 256;
                ret = p_wifi_interface->is_ap_connected(ssid, &len);
                if(ret){
                    printf("Connected ap %s\n", ssid);
                }
                
                ret = p_wifi_interface->disconnect_ap(event_label);
                if(ret < 0)
                {
                    printf("call error! check event\n");
                }else{
                    printf("disconnect ap ok!");
                }    
            }         
        }else{
            printf("call error! check event\n");
        }
    }else{
            	
        printf("WiFi isn't ready, waiting\n");
    }
    
    event_label++;
    p_wifi_interface->connect_ap_auto(event_label);


/********************************************************/
   
/********************************************************/
    while(1);
    return 0;
}