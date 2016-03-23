#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include "wifi.h"
#include "wifi_event.h"
#include "wifi_intf.h"

#define CMD_LEN        255
#define REPLY_BUF_SIZE 4096 // wpa_supplicant's maximum size.
#define NET_ID_LEN     4

extern int is_ip_exist();

int  gwifi_state = NETWORK_DISCONNECTED;
char netid_connecting[NET_ID_LEN+1] = {0};
static tWifi_event_callback event_handle = NULL;

static int docommand(char const *cmd, char *reply, size_t reply_len)
{
    if(!cmd || !cmd[0]){
        return -1;
    }
    
    printf("do cmd %s\n", cmd);
    
    --reply_len; // Ensure we have room to add NUL termination.
    if (wifi_command(cmd, reply, &reply_len) != 0) {
        return -1;
    }
    
    // Strip off trailing newline.
    if (reply_len > 0 && reply[reply_len-1] == '\n') {
        reply[reply_len-1] = '\0';
    } else {
        reply[reply_len] = '\0';
    }
    return 0;
}

int wifi_on(tWifi_event_callback pcb)
{
    int i = 0, ret = -1;
    
    event_handle = pcb;
    	
    ret = wifi_connect_to_supplicant();
    if(ret){
        printf("wpa_suppplicant not running!\n");
        wifi_start_supplicant(0);
        do{
        	usleep(300000);
        	ret = wifi_connect_to_supplicant();
        	if(!ret){
        	    printf("Connected to wpa_supplicant\n");
        	    break;
        	}
        	i++;
        }while(ret && i<10);
    }
    wifi_event_loop(pcb); 
     
    return 0;    	
}

int wifi_off()
{   
    wifi_close_supplicant_connection();
    wifi_stop_supplicant(0); 
    return 0;    	
}

/* 
 * 1. link to ap
 * 2. get ip addr 
 *
*/
int get_ap_connected(char *netid, int *len)
{
    int ret = -1;
    char cmd[CMD_LEN+1] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    char *p_c=NULL;
    
    strncpy(cmd, "LIST_NETWORKS", CMD_LEN);
    cmd[CMD_LEN] = '\0';
    
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do list networks error!\n");
        return -1;
    }
    
    if ((p_c=strstr(reply, "CURRENT")) != NULL){
        char *p_s=NULL, *p_e=NULL, *p=NULL; 
        p_e = strchr(p_c, '\n');
        if(p_e){
            *p_e = '\0';
        }
        
        p_s = strrchr(reply, '\n');
        p_s++;
        p = strtok(p_s, "\t");
        if(p){
            if(netid != NULL && *len > 0){
                strncpy(netid, p, *len-1);
                netid[*len-1] = '\0';
                *len = strlen(netid);    
            }
        }

        return 1;
    } else {
        return 0;
    }
    	
}

int is_ap_connected(char *ssid, int *len)
{
    int ret = -1;
    char cmd[CMD_LEN+1] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    char *p_c=NULL;
    
    strncpy(cmd, "LIST_NETWORKS", CMD_LEN);
    cmd[CMD_LEN] = '\0';
    
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do list networks error!\n");
        return -1;
    }
    
    if ((p_c=strstr(reply, "CURRENT")) != NULL){
        char *p_s=NULL, *p_e=NULL, *p=NULL; 
        p_e = strchr(p_c, '\n');
        if(p_e){
            *p_e = '\0';
        }
        
        p_s = strrchr(reply, '\n');
        p_s++;
        p = strtok(p_s, "\t");
        p = strtok(NULL, "\t");
        if(p){
            if(ssid != NULL && *len > 0){
                strncpy(ssid, p, *len-1);
                ssid[*len-1] = '\0';
                *len = strlen(ssid);    
            }
        }

        /* check ip exist */
        ret = is_ip_exist();
        if(ret > 0){
            return 1;
        }

        return 0;
    } else {
        return 0;
    }
    	
}


int ping()
{
    int ret = -1;
    char cmd[16] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    
    strncpy(cmd, "PING", 15);
    
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret || strcmp(reply, "PONG")){
        printf("do ping error!\n");
        return -1;
    }
}

int scan()
{
	  int ret = -1;
    char cmd[16] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    
    strncpy(cmd, "SCAN", 15);
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do scan error!\n");
        return -1;
    }
    
    return 0;
}

int get_scan_results(char *result, int *len)
{
    int ret = -1;
    char cmd[16] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    
    strncpy(cmd, "SCAN_RESULTS", 15);    
    cmd[15] = '\0';    
    
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do scan results error!\n");
        return -1;
    }
    
    if((result != NULL) && (*len > 0)){
        strncpy(result, reply, *len-1);    
    }
    
    return 0;
}
/*
 * get ap(ssid/key_mgmt) status in wpa_supplicant.conf
 * return
 * -1: not exist
 * 1:  exist but not connected
 * 3:  exist and connected; network id in buffer net_id  
*/
static int is_ap_exist(const char *ssid, tKEY_MGMT key_mgmt, char *net_id, int *len)
{
    int ret = -1;
    char cmd[256] = {0};
    char reply[REPLY_BUF_SIZE] = {0}, key_type[128], key_reply[128];
    char *p_c=NULL, *ptr=NULL;
    int flag = 0;
    
    if(!ssid || !ssid[0]){
        printf("Error: ssid is NULL!\n");
        return -1;
    }
    
    /* parse key_type */
    if(key_mgmt == WPA_PSK || key_mgmt == WPA2_PSK){
        strncpy(key_type, "WPA-PSK", 128);
    } else {
        strncpy(key_type, "NONE", 128);
    }
    
    strncpy(cmd, "LIST_NETWORKS", 255);
    cmd[255] = '\0';
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do list networks error!\n");
        return -1;
    }
    
    ptr = reply;
    while((p_c=strstr(ptr, ssid)) != NULL){
        char *p_s=NULL, *p_e=NULL, *p=NULL;
        
        flag = 0;
   
        p_e = strchr(p_c, '\n');
        if(p_e){
            *p_e = '\0';
        }
        p_s = strrchr(ptr, '\n');
        p_s++;
        
        if(strstr(p_s, "CURRENT")){
            flag = 2;
        }
        
        p = strtok(p_s, "\t");
        if(p){
            if(net_id != NULL && *len > 0){
                strncpy(net_id, p, *len-1);
                net_id[*len-1] = '\0';
                *len = strlen(net_id);    
            }
        }
        
        /* get key_mgmt */
        sprintf(cmd, "GET_NETWORK %s key_mgmt", net_id);
        cmd[255] = '\0';
        ret = docommand(cmd, key_reply, sizeof(key_reply));
        if(ret){
            printf("do get network %s key_mgmt error!\n", net_id);
            return -1;
        }
 
        printf("GET_NETWORK %s key_mgmt reply %s\n", net_id, key_reply);
        printf("key type %s\n", key_type);
 
        if(strcmp(key_reply, key_type) == 0){
            flag += 1;
            break;
        }

        if(p_e == NULL){
            break;
        }else{
            *p_e = '\n';
            ptr = p_e;
        }
    }
    
    return flag;
}

/*
 * ssid to netid
*/
static int ssid2netid(char *ssid, tKEY_MGMT key_mgmt, char *net_id, int *len)
{
    int ret = -1;
    char cmd[CMD_LEN + 1] = {0};
    char reply[REPLY_BUF_SIZE] = {0}, key_type[128], key_reply[128];
    char *p_c=NULL, *ptr=NULL;
    int flag = 0;

    /* parse key_type */
    if(key_mgmt == WPA_PSK || key_mgmt == WPA2_PSK){
        strncpy(key_type, "WPA-PSK", 128);
    } else {
        strncpy(key_type, "NONE", 128);
    }
    
    /* list ap in wpa_supplicant.conf */
    strncpy(cmd, "LIST_NETWORKS", 15);
    cmd[15] = '\0';
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do list networks error!\n");
        return -1;
    }
    
    ptr = reply;
    while((p_c=strstr(ptr, ssid)) != NULL){
        char *p_s=NULL, *p_e=NULL, *p_t=NULL;
        
        flag = 0;
   
        p_e = strchr(p_c, '\n');
        if(p_e){
            *p_e = '\0';
        }
        p_s = strrchr(ptr, '\n');
        p_s++;
        
        if(strstr(p_s, "CURRENT")){
            flag = 2;
        }
        
        p_t = strchr(p_s, '\t');
        if(p_t){
        	  int tmp = 0;
            tmp = p_t - p_s;
            if(tmp <= NET_ID_LEN){
               strncpy(net_id, p_s, tmp);
               net_id[tmp] = '\0';
            }
            *len = tmp;
        }
        
        /* get key_mgmt */
        sprintf(cmd, "GET_NETWORK %s key_mgmt", net_id);
        cmd[CMD_LEN] = '\0';
        ret = docommand(cmd, key_reply, sizeof(key_reply));
        if(ret){
            printf("do get network %s key_mgmt error!\n", net_id);
            return -1;
        }
 
        if(strcmp(key_reply, key_type) == 0){
            flag += 1;
            break;
        }

        if(p_e == NULL){
            break;
        }else{
            *p_e = '\n';
            ptr = p_e;
        }
    }
    
    return flag;    	
}

/*
 * Get max priority val in wpa_supplicant.conf
 * return
 *-1: error
 * 0: no network
 * >0: max val
 */
static int get_max_priority()
{
    int  ret = -1;
    int  val = -1, max_val = 0, len = 0;
    char cmd[CMD_LEN + 1] = {0}, reply[REPLY_BUF_SIZE] = {0}, priority[32] = {0};
    char net_id[NET_ID_LEN+1];
    char *p_n = NULL, *p_t = NULL; 
    
    /* list ap in wpa_supplicant.conf */
    strncpy(cmd, "LIST_NETWORKS", CMD_LEN);
    cmd[CMD_LEN] = '\0';
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do list networks error!\n");
        return -1;
    }
    
    p_n = strchr(reply, '\n');
    while(p_n != NULL){
    	  p_n++;
        if((p_t = strchr(p_n, '\t')) != NULL){
            len = p_t - p_n;
            if(len <= NET_ID_LEN){
               strncpy(net_id, p_n, len);
               net_id[len] = '\0';
            }
        }
        
        sprintf(cmd, "GET_NETWORK %s priority", net_id);
        ret = docommand(cmd, priority, sizeof(priority));
        if(ret){
            printf("do get network priority error!\n");
            return -1;
        }
        
        val = atoi(priority);
        if(val >= max_val){
            max_val = val;
        }
        
        p_n = strchr(p_n, '\n');
    }
    	
    return max_val;    	
}

int connect_ap_key_mgmt(const char *ssid, tKEY_MGMT key_mgmt, const char *passwd)
{
	  int i=0, ret = -1, len = 0, max_prio = -1;
	  char cmd[CMD_LEN+1] = {0};
	  char reply[REPLY_BUF_SIZE] = {0}, netid[NET_ID_LEN+1]={0};
	  
	  if(!ssid || !ssid[0]){
	      printf("Error: ssid is NULL!\n");
	      return -1;
	  }
	  
	  /* must disconnect */
	  disconnect_ap();
	  
	  /* check already exist or connected */
	  len = 3;
	  ret = is_ap_exist(ssid, key_mgmt, netid, &len);
	  if(ret == 1 || ret == 3){
	      //network is exist or connected
	      ;
	  } else { /* network is not exist */
	      /* add network */
    	  strncpy(cmd, "ADD_NETWORK", CMD_LEN);
    	  cmd[CMD_LEN] = '\0';
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do add network results error!\n");
            return -1;
        }
    	  strncpy(netid, reply, NET_ID_LEN);
    	  netid[NET_ID_LEN] = '\0';
    	  
    	  /* set network ssid */
    	  sprintf(cmd, "SET_NETWORK %s ssid \"%s\"", netid, ssid);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network ssid error!\n");
            return -1;
        }
	  }

	  /* no passwd */
	  if (key_mgmt == NONE){
	      /* set network no passwd */
    	  sprintf(cmd, "SET_NETWORK %s key_mgmt NONE", netid);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt error!\n");
            return -1;
        }
	  } else if(key_mgmt == WPA_PSK || key_mgmt == WPA2_PSK){
	      /* set network psk passwd */
	      sprintf(cmd,"SET_NETWORK %s key_mgmt WPA-PSK", netid);
	      ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt WPA-PSK error!\n");
            return -1;
        }
	      
    	  sprintf(cmd, "SET_NETWORK %s psk \"%s\"", netid, passwd);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network psk error!\n");
            return -1;
        }
	  } else if(key_mgmt == WEP){
        /* set network  key_mgmt none */
    	  sprintf(cmd, "SET_NETWORK %s key_mgmt NONE", netid);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt none error!\n");
            return -1;
        }
        
        /* set network wep_key0 */
    	  sprintf(cmd, "SET_NETWORK %s wep_key0 %s", netid, passwd);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network wep_key0 error!\n");
            return -1;
        }  	      
	  } else {
	      printf("Error: key mgmt is none!\n");
	      return -1;
	  }
	  
	  /* get max priority in wpa_supplicant.conf */
    max_prio =  get_max_priority();
    
    /* set priority for network */
    sprintf(cmd,"SET_NETWORK %s priority %d", netid, (max_prio+1)); 
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do set priority error!\n");
        return -1;
    }
	  
	  gwifi_state = NETWORK_DISCONNECTED;
	  /* select network */
	  sprintf(cmd, "ENABLE_NETWORK %s", netid);
	  ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do enable network error!\n");
        return -1;
    }
    
    /* save config */
	  sprintf(cmd, "%s", "SAVE_CONFIG");
	  ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do save config error!\n");
        return -1;
    }
    
    /* save netid */
    strcpy(netid_connecting, netid);
    
    /* reconnect */
	  sprintf(cmd, "%s", "RECONNECT");
	  ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do reconnect network error!\n");
        return -1;
    }
    
    /* check timeout */
    start_check_connect_timeout();
    
    return 0;
}

int connect_ap(const char *ssid, const char *passwd)
{
	  int  i = 0;
	  
	  if(!ssid || !ssid[0]){
	      printf("Error: ssid is NULL!\n");
	      return -1;
	  }
	  
	  /* no password */
	  if(!passwd || !passwd[0]){
	      connect_ap_key_mgmt(ssid, NONE, passwd);    
	  }else{
	      /* try WPA-PSK */
	      connect_ap_key_mgmt(ssid, WPA_PSK, passwd);
	      
	      /* wait for check status connected/disconnected */
	      i = 0;        
        do {
            usleep(500000);
            if (gwifi_state == PASSWORD_FAILED){
            	  printf("connect_ap exit: password failed!\n");
                return -1;	
            }
            
            if(gwifi_state == CONNECT_TIMEOUT){
                printf("connect_ap exit: connect timeout!\n");
                break;
            }	
            i++;  
        } while((gwifi_state != NETWORK_CONNECTED) && (i < 16)); 
	  		
	  		if (gwifi_state == NETWORK_CONNECTED){
	  		   printf("Wi-Fi connetcted ssid %s\n", ssid);
	  		   return 0;
	  		}
	  		
	  		/* cancel ssid+WPA-PSK saved config */
	  		remove_network(ssid, WPA_PSK);
	  		
	  		/* try WEP */
	  		connect_ap_key_mgmt(ssid, WEP, passwd);
	  		
	  		usleep(5000*1000);
	  }
    
    return 0;    	
}

/* cancel saved AP in wpa_supplicant.conf */
int remove_network(char *ssid, tKEY_MGMT key_mgmt)
{
    int ret = -1, len = 0;
    char cmd[CMD_LEN+1] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    char net_id[NET_ID_LEN+1] = {0};

    if(!ssid || !ssid[0]){
        printf("Error: ssid is null!\n");
        return -1;
    }
    
    /* check AP is exist in wpa_supplicant.conf */
    len = NET_ID_LEN+1;
    ret = ssid2netid(ssid, key_mgmt, net_id, &len);
    if(ret <= 0){
        printf("Warning: %s not in wpa_supplicant.conf!\n", ssid);
        return 0;
    }
    
    /* cancel saved in wpa_supplicant.conf */
    sprintf(cmd, "REMOVE_NETWORK %s", net_id);
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do remove network %s error!\n", net_id);
        return -1;
    }
    
    /* save config */
	  sprintf(cmd, "%s", "SAVE_CONFIG");
	  ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do save config error!\n");
        return -1;
    }
    
    return 0;
}

int remove_all_networks()
{
    int ret = -1, len = 0;
    char cmd[CMD_LEN+1] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    char net_id[NET_ID_LEN+1] = {0};
    char *p_n = NULL, *p_t = NULL;
    
    /* list ap in wpa_supplicant.conf */
    strncpy(cmd, "LIST_NETWORKS", CMD_LEN);
    cmd[CMD_LEN] = '\0';
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do list networks error!\n");
        return -1;
    }
    
    p_n = strchr(reply, '\n');
    while(p_n != NULL){
    	  p_n++;
        if((p_t = strchr(p_n, '\t')) != NULL){
            len = p_t - p_n;
            if(len <= NET_ID_LEN){
               strncpy(net_id, p_n, len);
               net_id[len] = '\0';
            }
        }
        
        /* cancel saved in wpa_supplicant.conf */
        sprintf(cmd, "REMOVE_NETWORK %s", net_id);
        ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do remove network %s error!\n", net_id);
            return -1;
        }
        
        p_n = strchr(p_n, '\n');
    }
    
    /* save config */
	  sprintf(cmd, "%s", "SAVE_CONFIG");
	  ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do save config error!\n");
        return -1;
    }
    
    return 0;
}

int connect_ap_auto()
{
    int i=0, ret = -1;
    char cmd[CMD_LEN+1] = {0}, reply[REPLY_BUF_SIZE] = {0};
    char netid[NET_ID_LEN+1]={0};
    int len = 0;

    netid_connecting[0] = '\0';

    /* reconnected */
	  sprintf(cmd, "%s", "RECONNECT");
	  ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do reconnect network error!\n");
        return -1;
    }
    
    return 0;    	
}

static int get_netid_connected(char *net_id, int *len)
{
    int ret = -1;
    char cmd[CMD_LEN+1] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    char *p_c=NULL;
    
    strncpy(cmd, "LIST_NETWORKS", CMD_LEN);
    cmd[CMD_LEN] = '\0';
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do list networks error!\n");
        return -1;
    }
    
    if ((p_c=strstr(reply, "CURRENT")) != NULL){
        char *p_s=NULL, *p_e=NULL, *p=NULL; 
        p_e = strchr(p_c, '\n');
        if(p_e){
            *p_e = '\0';
        }
        
        p_s = strrchr(reply, '\n');
        p_s++;
        p = strtok(p_s, "\t");
        if(p){
            if(net_id != NULL && *len > 0){
                strncpy(net_id, p, *len-1);
                net_id[*len-1] = '\0';
                *len = strlen(net_id);    
            }
        }

        return 1;
    } else {
        return 0;
    }
    	
}

int disconnect_ap()
{
    int i=0, ret = -1;
    char cmd[CMD_LEN+1] = {0}, reply[REPLY_BUF_SIZE] = {0};
    char netid[NET_ID_LEN+1]={0};
    int len = 0;
    
    len = NET_ID_LEN+1;
    ret = get_netid_connected(netid, &len);
    if(ret <= 0){
        printf("This no connected AP!\n");	
        return 0;
    }
    
    /* disconnected */
	  sprintf(cmd, "%s", "DISCONNECT");
	  ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do disconnect network error!\n");
        return -1;
    }
    
    i=0;
    do{
        usleep(300000);
        if(gwifi_state == NETWORK_DISCONNECTED){
            break;
        }
        i++;	
    }while(i<10);
    
    return 0;    	
}
