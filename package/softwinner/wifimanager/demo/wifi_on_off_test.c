#define TAG "wifi"
#include <tina_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <errno.h>
#include <unistd.h>
#include <wifi_intf.h>
#include <pthread.h>

#define WIFI_ON_OFF_TEST_CNTS  1000

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
    int ret = 0;
    int i = 0, event_label = 0;

    const aw_wifi_interface_t *p_wifi_interface = NULL;
    
    event_label=rand();
    for(i=0; i<WIFI_ON_OFF_TEST_CNTS;i++)
    {
        printf("\n***************************\n");
        printf("Do wifi on off test %d times\n", i);
        printf("****************************\n");

        event_label++;
        /* turn on wifi */
        p_wifi_interface = aw_wifi_on(wifi_event_handle, event_label);
        if(p_wifi_interface == NULL)
        {
    	    printf("Test failed: wifi on failed with event 0x%x\n", event);
            return -1;
        }
        
        /* test */
        usleep(10000);

        /* turn off wifi */
        ret = aw_wifi_off(p_wifi_interface);
        if(ret < 0)
        {
            printf("Test failed: wifi off error!\n");
            return -1;
        }

        /* wait event */
        while(event != WIFIMG_WIFI_OFF_SUCCESS)
        {
            usleep(200);
        }
    }

    printf("********************************\n");
    printf("Test completed: Success!\n");
    printf("********************************\n");
    
    return 0;
}
