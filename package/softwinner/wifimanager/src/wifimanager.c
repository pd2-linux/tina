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
 * get ap(ssid) status in wpa_supplicant.conf
 * return
 * -1: not exist
 * 1:  exist but not connected
 * 3:  exist and connected; network id in buffer net_id  
*/
static int is_ap_exist(const char *ssid, char *net_id, int *len)
{
    int ret = -1;
    char cmd[16] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    char *p_c=NULL;
    int flag = 0;
    
    if(!ssid || !ssid[0]){
        printf("Error: ssid is NULL!\n");
        return -1;
    }
    
    strncpy(cmd, "LIST_NETWORKS", 15);
    cmd[15] = '\0';
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do list networks error!\n");
        return -1;
    }
    
    if ((p_c=strstr(reply, ssid)) != NULL){
        flag = 1;    	 

        char *p_s=NULL, *p_e=NULL, *p=NULL; 
        p_e = strchr(p_c, '\n');
        if(p_e){
            *p_e = '\0';
        }
        p_s = strrchr(reply, '\n');
        p_s++;
        
        if(strstr(p_s, "CURRENT")){
            flag += 2;
        }
        
        p = strtok(p_s, "\t");
        if(p){
            if(net_id != NULL && *len > 0){
                strncpy(net_id, p, *len-1);
                net_id[*len-1] = '\0';
                *len = strlen(net_id);    
            }
        }
        
        return flag;
    } else {
        return 0;
    }
    	
}

/*
 * ssid to netid
*/
static int ssid2netid(char *ssid, char *net_id, int *len)
{
    int ret = -1;
    char cmd[16] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    char *p_c = NULL;
    
    /* list ap in wpa_supplicant.conf */
    strncpy(cmd, "LIST_NETWORKS", 15);
    cmd[15] = '\0';
    ret = docommand(cmd, reply, sizeof(reply));
    if(ret){
        printf("do list networks error!\n");
        return -1;
    }
    
    p_c = strstr(reply, ssid);
    if(p_c != NULL){
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
    }else{
        return 0;
    }    	
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
    char cmd[CMD_LEN + 1] = {0}, reply[REPLY_BUF_SIZE] = {0};
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
        ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do get network priority error!\n");
            return -1;
        }
        
        val = atoi(reply);
        if(val >= max_val){
            max_val = val;
        }
        
        p_n = strchr(reply, '\n');
    }
    	
    return max_val;    	
} 


int connect_ap(const char *ssid, const char *passwd)
{
    int ret = -1;
	  char cmd[CMD_LEN + 1] = {0};
	  char reply[REPLY_BUF_SIZE] = {0}, netid[NET_ID_LEN+1]={0};
	  int  i = 0, len = 0, max_prio = -1;
	  
	  if(!ssid || !ssid[0]){
	      printf("Error: ssid is NULL!\n");
	      return -1;
	  }
	  
	  /* must disconnect */
	  disconnect_ap();
	  
	  /* check already exist or connected */
	  len = 3;
	  ret = is_ap_exist(ssid, netid, &len);
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
	  if (!passwd || !passwd[0]){
	      /* set network no passwd */
    	  sprintf(cmd, "SET_NETWORK %s key_mgmt NONE", netid);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt error!\n");
            return -1;
        }   
	  } else {	
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
        /* enable network */
    	  sprintf(cmd, "ENABLE_NETWORK %s", netid);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do select network error!\n");
            return -1;
        }
        
        /* save config */
    	  sprintf(cmd, "%s", "SAVE_CONFIG");
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do save config error!\n");
            return -1;
        }
        
        i = 0;        
        do {
            usleep(100000);
            if (gwifi_state == PASSWORD_FAILED){
            	  printf("Exit: password failed!\n");
                return -1;	
            }	
            i++;  
        } while((gwifi_state != NETWORK_CONNECTED) && (i < 30)); 
	  		
	  		if (gwifi_state == NETWORK_CONNECTED){
	  		   printf("Wi-Fi connetcted ssid %s\n", ssid);
	  		   return 0;
	  		}
	  		
	  		/* disable network */
	  		sprintf(cmd, "DISABLE_NETWORK %s", netid);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do disable network error!\n");
            return -1;
        }
	  		  	
	  	  /* set network key_mgmt none */
    	  sprintf(cmd, "SET_NETWORK %s key_mgmt NONE", netid);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt none error!\n");
            return -1;
        }
        
        /* set network  key_mgmt none */
    	  sprintf(cmd, "SET_NETWORK %s wep_key0 %s", netid, passwd);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network wep_key0 error!\n");
            return -1;
        }
	  }

    gwifi_state = NETWORK_DISCONNECTED;
    /* enable network */
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
    
    /* reconnect */
    connect_ap_auto();
    
    /* check timeout */
    start_check_connect_timeout();
    
    return 0;    	
}

int connect_ap_key_mgmt(const char *ssid, tKEY_MGMT key_mgnt, const char *passwd)
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
	  ret = is_ap_exist(ssid, netid, &len);
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
	  if (key_mgnt == NONE){
	      /* set network no passwd */
    	  sprintf(cmd, "SET_NETWORK %s key_mgmt NONE", netid);
    	  ret = docommand(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt error!\n");
            return -1;
        }
	  } else if(key_mgnt == WPA_PSK || key_mgnt == WPA2_PSK){
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
	  } else if(key_mgnt == WEP){
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
    
    /* reconnect */
    connect_ap_auto();
    
    /* check timeout */
    start_check_connect_timeout();
    
    return 0;
}

/* cancel saved AP in wpa_supplicant.conf */
int cancel_saved(char *ssid)
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
    ret = ssid2netid(ssid, net_id, &len);
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

int connect_ap_auto()
{
    int i=0, ret = -1;
    char cmd[CMD_LEN+1] = {0}, reply[REPLY_BUF_SIZE] = {0};
    char netid[NET_ID_LEN+1]={0};
    int len = 0;

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
