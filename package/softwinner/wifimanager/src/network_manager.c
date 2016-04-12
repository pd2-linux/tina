#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

#include "wifi_event.h"
#include "network_manager.h"
#include "wifi_intf.h"

static char scan_results[SCAN_BUF_LEN];
static int  scan_results_len = 0;
static int  scan_running = 0;
static pthread_t       scan_thread_id;
static pthread_mutex_t scan_mutex;

int update_scan_results()
{
    int ret = -1, i;
    char cmd[16] = {0}, reply[16] = {0};
    
    pthread_mutex_lock(&scan_mutex);
    
    /* set scan start flag */
    set_scan_start_flag();
    
    /* scan cmd */
    strncpy(cmd, "SCAN", 15);
    ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do scan error!\n");
        return -1;
    }
    
    while (get_scan_status() != 1);
    
    strncpy(cmd, "SCAN_RESULTS", 15);
    cmd[15] = '\0';
    ret = wifi_command(cmd, scan_results, sizeof(scan_results));
    if(ret){
        printf("do scan results error!\n");
        return -1;
    }
    scan_results_len =  strlen(scan_results);
    
    pthread_mutex_unlock(&scan_mutex);
    
    return 0;
}

int get_scan_results_inner(char *result, int *len)
{
	  int index = 0;
	  char *ptr = NULL;
	  
	  pthread_mutex_lock(&scan_mutex);
	  
    if(*len <= scan_results_len){
        strncpy(result, scan_results, *len-1);
        index = *len -1;
        result[index] = '\0';
        ptr=strrchr(result, '\n');
        if(ptr != NULL){
            *ptr = '\0';
        }
    }else{
        strncpy(result, scan_results, scan_results_len);
        result[scan_results_len] = '\0';
    }
    
    pthread_mutex_unlock(&scan_mutex);
    
    return 0;
}

int is_network_exist(const char *ssid, tKEY_MGMT key_mgmt)
{
    int ret = 0, i = 0, key[4] = {0};
    
    for(i=0; i<4; i++){
        key[i]=0;
    }
    
    get_key_mgmt(ssid, key);
    if(key_mgmt == WIFIMG_NONE){
        if(key[0] == 1){
            ret = 1;
        }
    }else if(key_mgmt == WIFIMG_WPA_PSK || key_mgmt == WIFIMG_WPA2_PSK){
        if(key[1] == 1){
            ret = 1;
        }
    }else if(key_mgmt == WIFIMG_WEP){
        if(key[2] == 1){
            ret = 1;
        }
    }else{
        ;
    }
    
    return ret;
}

int get_key_mgmt(const char *ssid, int key_mgmt_info[])
{
    char *ptr = NULL, *pssid = NULL;
    char *pst = NULL, *pend = NULL;
    char *pflag = NULL;
    char flag[128];
    int  len = 0, i = 0;
    
    printf("enter get_key_mgmt\n");
    
    key_mgmt_info[KEY_NONE_INDEX] = 0;    	
    key_mgmt_info[KEY_WEP_INDEX] = 0;    	
    key_mgmt_info[KEY_WPA_PSK_INDEX] = 0;
    
    pthread_mutex_lock(&scan_mutex);
    
    /* first line end */
    ptr = strchr(scan_results, '\n');
    if(!ptr){
        pthread_mutex_unlock(&scan_mutex);
        return 0;
    }

    //point second line of scan results
    ptr++;
    while((pssid=strstr(ptr, ssid)) != NULL){
        /* line end */
        pend = strchr(pssid, '\n');
        if(pend != NULL){
            *pend = '\0';
        }
        
        pst = strrchr(ptr, '\n');
        if(pst != NULL){
            pst++;
        }else{
            pst = ptr;
        }
        
        pflag = pst;
        for(i=0; i<3; i++){
            pflag = strchr(pflag, '\t');
            pflag++;
        }
        
        len = pssid - pflag;
        len = len - 1;
        strncpy(flag, pflag, len);
        flag[len] = '\0';
        printf("ssid %s, flag %s\n", ssid, flag);
        
        if((strstr(flag, "WPA-PSK-") != NULL) 
            || (strstr(flag, "WPA2-PSK-") != NULL)){
            key_mgmt_info[KEY_WPA_PSK_INDEX] = 1;
        }else if(strstr(flag, "WEP") != NULL){
            key_mgmt_info[KEY_WEP_INDEX] = 1;
        }else if((strcmp(flag, "[ESS]") == 0) || (strcmp(flag, "[WPS][ESS]") == 0)){
            key_mgmt_info[KEY_NONE_INDEX] = 1;
        }else{
            ;
        }
        
        if(pend != NULL){
            *pend = '\n';
            //point next line
            ptr = pend+1;
        }else{
            break;
        }
    }
    
    pthread_mutex_unlock(&scan_mutex);
    
    printf("exit get_key_mgmt\n");
    
    return 0;
    
    
}

void *wifi_scan_thread(void *args)
{
    int ret = -1, i = 0;
    char cmd[16] = {0}, reply[16] = {0};
    
    while(scan_running){
     
        pthread_mutex_lock(&scan_mutex);
        
        /* set scan start flag */
        set_scan_start_flag();
        
        /* scan cmd */
        strncpy(cmd, "SCAN", 15);
        ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            pthread_mutex_unlock(&scan_mutex);
            usleep(5000*1000);
            continue;
        }
        
        while (get_scan_status() != 1);
        
        strncpy(cmd, "SCAN_RESULTS", 15);
        cmd[15] = '\0';
        ret = wifi_command(cmd, scan_results, sizeof(scan_results));
        if(ret){
            printf("do scan results error!\n");
            pthread_mutex_unlock(&scan_mutex);
            continue;
        }
        scan_results_len =  strlen(scan_results);
        
        pthread_mutex_unlock(&scan_mutex);
        
        usleep(15000*1000);
    }
}

void start_wifi_scan_thread(void *args)
{
	  scan_running = 1;
    pthread_create(&scan_thread_id, NULL, &wifi_scan_thread, args);
    pthread_mutex_init(&scan_mutex, NULL);
}

void stop_wifi_scan_thread()
{
	  scan_running = 0;
	  usleep(200*1000);
	  pthread_join(scan_thread_id, NULL);
	  
	  pthread_mutex_destroy(&scan_mutex);
}