#define TAG "wifi"
#include <tina_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <errno.h>
#include <unistd.h>
#include <wifi_intf.h>

static void wifi_event_handle(tWIFI_STATE wifi_state, void *buf)
{
    switch(wifi_state)
    {
        case NETWORK_CONNECTED:
        {
            printf("WiFi connected ap!\n");
            break;	
        }
        
        case NETWORK_DISCONNECTED:
        {
            printf("WiFi disconnected!\n");
            break;	
        }
        
        case PASSWORD_FAILED:
        {
            printf("Password authentication failed!\n");
            break;	
        }
        
        case CONNECT_TIMEOUT:
        {
            printf("Connected timeout!\n");
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
    char ssid[256] = {0}, scan_results[4096] = {0};
    
    if(wifi_on(wifi_event_handle)){
        return -1;
    }
    
    scan();
    usleep(2000000);
    len = 4096;
    get_scan_results(scan_results, &len);
    printf("scan results:\n");
    printf("%s\n", scan_results);
    
    connect_ap_key_mgmt(argc[1], WPA_PSK, argc[2]);
    //connect_ap(argc[1], argc[2]);
    usleep(1000000);
    
    len = 256;
    ret = is_ap_connected(ssid, &len);
    if(ret){
        printf("Connected ap %s\n", ssid);
    }
    
    disconnect_ap();
    usleep(1000000);
    
    connect_ap_auto();
    
/********************************************************/
   
/********************************************************/
    while(1);
    return 0;
}