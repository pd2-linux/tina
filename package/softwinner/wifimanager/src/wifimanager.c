#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>

#include "wifi.h"
#include "wifi_event.h"
#include "wifi_state_machine.h"
#include "network_manager.h"
#include "wpa_supplicant_conf.h"
#include "wifi_intf.h"

extern int is_ip_exist();

char netid_connecting[NET_ID_LEN+1] = {0};
int  disconnecting = 0;
int  connecting_ap_event_label = 0;
int  disconnect_ap_event_label = 0;
tWIFI_STATE  gwifi_state = WIFIMG_WIFI_DISABLED;

static tWIFI_EVENT  event_code = WIFIMG_NO_NETWORK_CONNECTING;
static int aw_wifi_disconnect_ap(int event_label);

static int aw_wifi_add_event_callback(tWifi_event_callback pcb)
{
      return add_wifi_event_callback_inner(pcb);
}

static int aw_wifi_is_ap_connected(char *ssid, int *len)
{
    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return -1;
    }
    	
    return wpa_conf_is_ap_connected(ssid, len);
}

static int aw_wifi_scan()
{
    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return -1;
    }

    update_scan_results();
    return 0;
}

static int aw_wifi_get_scan_results(char *result, int *len)
{
    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return -1;
    }
    	
    get_scan_results_inner(result, len);
    return 0;
}

/* connect visiable network */
static int aw_wifi_add_network(const char *ssid, tKEY_MGMT key_mgmt, const char *passwd, int event_label)
{
    int i=0, ret = -1, len = 0, max_prio = -1;
    char cmd[CMD_LEN+1] = {0};
    char reply[REPLY_BUF_SIZE] = {0}, netid[NET_ID_LEN+1]={0};
    tWIFI_MACHINE_STATE wifi_machine_state;
	  
    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return -1;
    }
	  
    if(!ssid || !ssid[0]){
        printf("Error: ssid is NULL!\n");
        ret = -1;
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        goto end;
    }
	  
    wifi_machine_state = get_wifi_machine_state();
    if(wifi_machine_state != CONNECTED_STATE && wifi_machine_state != DISCONNECTED_STATE){
        ret = -1;
        event_code = WIFIMG_DEV_BUSING_EVENT;
        goto end;
    }
    
    /* ensure disconnected */
    wifi_machine_state = get_wifi_machine_state();
    if(wifi_machine_state == CONNECTED_STATE){
        aw_wifi_disconnect_ap(0x7fffffff);
    }
    
    /* connecting */
    set_wifi_machine_state(CONNECTING_STATE);
	  
    /* set connecting event label */
    connecting_ap_event_label = event_label;
	  
    /* remove disconnecting flag */
    disconnecting = 0;
	  
    /* check already exist or connected */
    len = 3;
    ret = wpa_conf_is_ap_exist(ssid, key_mgmt, netid, &len);
    if(ret == 1 || ret == 3){
        //network is exist or connected
        ;
    } else { /* network is not exist */
	    /* add network */
    	strncpy(cmd, "ADD_NETWORK", CMD_LEN);
    	cmd[CMD_LEN] = '\0';
    	ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do add network results error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
    	strncpy(netid, reply, NET_ID_LEN);
    	netid[NET_ID_LEN] = '\0';
    	  
    	/* set network ssid */
    	sprintf(cmd, "SET_NETWORK %s ssid \"%s\"", netid, ssid);
    	ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network ssid error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
	}

	  /* no passwd */
	  if (key_mgmt == WIFIMG_NONE){
	      /* set network no passwd */
    	  sprintf(cmd, "SET_NETWORK %s key_mgmt NONE", netid);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
	  } else if(key_mgmt == WIFIMG_WPA_PSK || key_mgmt == WIFIMG_WPA2_PSK){
	      /* set network psk passwd */
	      sprintf(cmd,"SET_NETWORK %s key_mgmt WPA-PSK", netid);
	      ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt WPA-PSK error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
	      
    	  sprintf(cmd, "SET_NETWORK %s psk \"%s\"", netid, passwd);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network psk error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
	  } else if(key_mgmt == WIFIMG_WEP){
        /* set network  key_mgmt none */
    	  sprintf(cmd, "SET_NETWORK %s key_mgmt NONE", netid);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt none error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
        
        /* set network wep_key0 */
    	  sprintf(cmd, "SET_NETWORK %s wep_key0 %s", netid, passwd);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network wep_key0 error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }

        /* set network auth_alg */
    	  sprintf(cmd, "SET_NETWORK %s auth_alg OPEN SHARED", netid);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network auth_alg error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
  	      
	  } else {
	      printf("Error: key mgmt not support!\n");
	      ret = -1;
	      event_code = WIFIMG_KEY_MGMT_NOT_SUPPORT;
	      goto end;
	  }
	  
	  /* get max priority in wpa_supplicant.conf */
    max_prio =  wpa_conf_get_max_priority();
    
    /* set priority for network */
    sprintf(cmd,"SET_NETWORK %s priority %d", netid, (max_prio+1)); 
    ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do set priority error!\n");
        ret = -1;
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        goto end;
    }
	  
	  /* select network */
	  sprintf(cmd, "SELECT_NETWORK %s", netid);
	  ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do select network error!\n");
        ret = -1;
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        goto end;
    }
    
    /* save config */
	  sprintf(cmd, "%s", "SAVE_CONFIG");
	  ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do save config error!\n");
        ret = -1;
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        goto end;
    }
    
    /* save netid */
    strcpy(netid_connecting, netid);
    
    /* reconnect */
	  sprintf(cmd, "%s", "RECONNECT");
	  ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do reconnect network error!\n");
        ret = -1;
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        goto end;
    }
    
    /* check timeout */
    start_check_connect_timeout(0);

end:
    if(ret != WIFI_MANAGER_SUCCESS){
        call_event_callback_function(event_code, NULL, event_label);
    }
    
    return ret;
}

static int wifi_connect_ap_inner(const char *ssid, tKEY_MGMT key_mgmt, const char *passwd, int event_label)
{
    int i=0, ret = -1, len = 0, max_prio = -1;
	  char cmd[CMD_LEN+1] = {0};
	  char reply[REPLY_BUF_SIZE] = {0}, netid1[NET_ID_LEN+1]={0}, netid2[NET_ID_LEN+1] = {0};
    int is_exist = 0;
    tWIFI_MACHINE_STATE  state;
	  tWIFI_EVENT_INNER    event;
    
	  /* connecting */
	  set_wifi_machine_state(CONNECTING_STATE);
	  
	  /* set connecting event label */
	  connecting_ap_event_label = event_label;
	  
	  /* remove disconnecting flag */
	  disconnecting = 0;
	  
	  /* check already exist or connected */
	  len = 3;
	  is_exist = wpa_conf_is_ap_exist(ssid, key_mgmt, netid1, &len);
	  if(is_exist == 1 || is_exist == 3){
	      //network is exist or connected
/*	      
	      sprintf(cmd, "DISABLE_NETWORK %s", netid1);
        cmd[CMD_LEN] = '\0';
        ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do disable network error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
*/
	  }
	  
    /* add network */
    strncpy(cmd, "ADD_NETWORK", CMD_LEN);
    cmd[CMD_LEN] = '\0';
    ret = wifi_command(cmd, netid2, sizeof(netid2));
    if(ret){
        printf("do add network results error!\n");
        ret = -1;
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        goto end;
    }
	  
	  /* set network ssid */
	  sprintf(cmd, "SET_NETWORK %s ssid \"%s\"", netid2, ssid);
	  ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do set network ssid error!\n");
        ret = -1;
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        goto end;
    }
	  
	  /* no passwd */
	  if (key_mgmt == WIFIMG_NONE){
	      /* set network no passwd */
    	  sprintf(cmd, "SET_NETWORK %s key_mgmt NONE", netid2);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt error!\n");
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            ret = -1;
            goto end;
        }
	  } else if(key_mgmt == WIFIMG_WPA_PSK || key_mgmt == WIFIMG_WPA2_PSK){
	      /* set network psk passwd */
	      sprintf(cmd,"SET_NETWORK %s key_mgmt WPA-PSK", netid2);
	      ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt WPA-PSK error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
	      
    	  sprintf(cmd, "SET_NETWORK %s psk \"%s\"", netid2, passwd);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network psk error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
	  } else if(key_mgmt == WIFIMG_WEP){
        /* set network  key_mgmt none */
    	  sprintf(cmd, "SET_NETWORK %s key_mgmt NONE", netid2);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network key_mgmt none error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }
        
        /* set network wep_key0 */
    	  sprintf(cmd, "SET_NETWORK %s wep_key0 %s", netid2, passwd);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network wep_key0 error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }

        /* set network auth_alg */
    	  sprintf(cmd, "SET_NETWORK %s auth_alg OPEN SHARED", netid2);
    	  ret = wifi_command(cmd, reply, sizeof(reply));
        if(ret){
            printf("do set network auth_alg error!\n");
            ret = -1;
            event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
            goto end;
        }

	  } else {
	      printf("Error: key mgmt is not support!\n");
	      ret = -1;
	      event_code = WIFIMG_KEY_MGMT_NOT_SUPPORT;
	      goto end;
	  }
	  
	  /* get max priority in wpa_supplicant.conf */
    max_prio =  wpa_conf_get_max_priority();
    
    /* set priority for network */
    sprintf(cmd,"SET_NETWORK %s priority %d", netid2, (max_prio+1));
    ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do set priority error!\n");
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        ret = -1;
        goto end;
    }
	  
	  /* select network */
	  sprintf(cmd, "SELECT_NETWORK %s", netid2);
	  ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do select network error!\n");
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        ret = -1;
        goto end;
    }
    
    /* save config */
	  sprintf(cmd, "%s", "SAVE_CONFIG");
	  ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do save config error!\n");
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        ret = -1;
        goto end;
    }
    
    /* save netid */
    strcpy(netid_connecting, netid2);
    
    /* reconnect */
	  sprintf(cmd, "%s", "RECONNECT");
	  ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do reconnect network error!\n");
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        ret = -1;
        goto end;
    }
    
    /* check timeout */
    start_check_connect_timeout(0);
    
    /* wait for check status connected/disconnected */
    i = 0;
    do {
        usleep(500000);
        
        state = get_wifi_machine_state();
        event = get_cur_wifi_event();
        /* password incorrect*/
        if ((state == DISCONNECTED_STATE) && (event == PASSWORD_INCORRECT)){
        	  printf("wifi_connect_ap_inner: password failed!\n");
            break;
        }

        i++;
    } while((state != L2CONNECTED_STATE) && (state != CONNECTED_STATE) && (i < 45));
    
    if(state == DISCONNECTED_STATE){
        if(event == PASSWORD_INCORRECT || event == CONNECT_AP_TIMEOUT){
            /* disable network in wpa_supplicant.conf */
            sprintf(cmd, "DISABLE_NETWORK %s", netid2);
            wifi_command(cmd, reply, sizeof(reply));
            
            /* cancel saved in wpa_supplicant.conf */
            sprintf(cmd, "REMOVE_NETWORK %s", netid2);
            wifi_command(cmd, reply, sizeof(reply));
            
            if(is_exist == 1 || is_exist == 3){
/*
        	      //network is exist or connected
        	      sprintf(cmd, "ENABLE_NETWORK %s", netid1);
                cmd[CMD_LEN] = '\0';
                wifi_command(cmd, reply, sizeof(reply));
*/
	          }
	          
	          /* save config */
        	  sprintf(cmd, "%s", "SAVE_CONFIG");
        	  wifi_command(cmd, reply, sizeof(reply));
            
            ret = -1;
            event_code = WIFIMG_NETWORK_NOT_EXIST;
            goto end;
        }else if(event == OBTAINING_IP_TIMEOUT){
            if(is_exist == 1 || is_exist == 3){
        	      //network is exist or connected
        	      sprintf(cmd, "REMOVE_NETWORK %s", netid1);
                cmd[CMD_LEN] = '\0';
                wifi_command(cmd, reply, sizeof(reply));
                
                /* save config */
            	  sprintf(cmd, "%s", "SAVE_CONFIG");
            	  wifi_command(cmd, reply, sizeof(reply));
	          }
	          
            ret = 0;
        }else{
            ret = -1;
        }
    }else if(state == L2CONNECTED_STATE || state == CONNECTED_STATE){
        if(is_exist == 1 || is_exist == 3){
    	      //network is exist or connected
    	      sprintf(cmd, "REMOVE_NETWORK %s", netid1);
            cmd[CMD_LEN] = '\0';
            wifi_command(cmd, reply, sizeof(reply));
            
            /* save config */
        	  sprintf(cmd, "%s", "SAVE_CONFIG");
        	  wifi_command(cmd, reply, sizeof(reply));
        }

        ret = 0;
    }else{
        ret = -1;
    }
    
end:
    //enable all networks in wpa_supplicant.conf
    wpa_conf_enable_all_networks();
	
    //restore state when call wrong
    if(ret != 0){
        set_wifi_machine_state(DISCONNECTED_STATE);
        event_code = WIFIMG_NETWORK_NOT_EXIST;
        return ret;
    }
    
    return ret;
}

/* connect visiable network */
static int aw_wifi_connect_ap_key_mgmt(const char *ssid, tKEY_MGMT key_mgmt, const char *passwd, int event_label)
{
	  int ret = -1, key[4] = {0};
	  tWIFI_MACHINE_STATE  state;
	  
	  if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return -1;
    }
	  
	  if(!ssid || !ssid[0]){
	      printf("Error: ssid is NULL!\n");
	      ret = -1;
	      event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
	      goto end;
	  }
	  
	  state = get_wifi_machine_state();
	  if(state != CONNECTED_STATE && state != DISCONNECTED_STATE){
        ret = -1;
        event_code = WIFIMG_DEV_BUSING_EVENT;
        goto end;
    }
    
    /* checking network exist at first time */
	  get_key_mgmt(ssid, key);
	  
	  /* no password */
	  if (key_mgmt == WIFIMG_NONE){
        if(key[0] == 0){
            update_scan_results();
            get_key_mgmt(ssid, key);
            if(key[0] == 0){
                ret = -1;
                event_code = WIFIMG_NETWORK_NOT_EXIST;
                goto end;
            }
        }
	  }else if(key_mgmt == WIFIMG_WPA_PSK || key_mgmt == WIFIMG_WPA2_PSK){
        if(key[1] == 0){
            update_scan_results();
            get_key_mgmt(ssid, key);
            if(key[1] == 0){
                ret = -1;
                event_code = WIFIMG_NETWORK_NOT_EXIST;
                goto end;
            }
        }
    }else if(key_mgmt == WIFIMG_WEP){
        if(key[2] == 0){
            update_scan_results();
            get_key_mgmt(ssid, key);
            if(key[2] == 0){
                ret = -1;
                event_code = WIFIMG_NETWORK_NOT_EXIST;
                goto end;
            }
        }
    }else{
        ret = -1;
        event_code = WIFIMG_KEY_MGMT_NOT_SUPPORT;
        goto end;
    }
    
    /* ensure wifi disconnect */
    state = get_wifi_machine_state();
    if(state == CONNECTED_STATE){
        aw_wifi_disconnect_ap(0x7fffffff);
    }
    
    ret = wifi_connect_ap_inner(ssid, key_mgmt, passwd, event_label);

end:
    if(ret != 0){
        call_event_callback_function(event_code, NULL, event_label);
    }
    return ret;
}


static int aw_wifi_connect_ap(const char *ssid, const char *passwd, int event_label)
{
	  int  i = 0, ret = 0;
	  int  key[4] = {0};
	  tWIFI_MACHINE_STATE  state;
	  tWIFI_EVENT_INNER    event;

    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        printf("aw wifi connect ap wifi disabled\n");
        return -1;
    }

	  if(!ssid || !ssid[0]){
	      printf("Error: ssid is NULL!\n");
	      ret = -1;
	      event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
	      goto end;
	  }

	  state = get_wifi_machine_state();
          printf("aw wifi connect state 0x%x\n", state);
	  if(state != CONNECTED_STATE && state != DISCONNECTED_STATE){
        ret = -1;
        printf("aw wifi connect ap dev busing\n");
        event_code = WIFIMG_DEV_BUSING_EVENT;
        goto end;
    }

	  /* checking network exist at first time */
	  get_key_mgmt(ssid, key);
	  
	  /* no password */
	  if(!passwd || !passwd[0]){
        if(key[0] == 0){
            update_scan_results();
            get_key_mgmt(ssid, key);
            if(key[0] == 0){
                ret = -1;
                event_code = WIFIMG_NETWORK_NOT_EXIST;
                goto end;
            }
        }
        
        /* ensure disconnected */
        state = get_wifi_machine_state();
        if (state == CONNECTED_STATE){
            aw_wifi_disconnect_ap(0x7fffffff);
        }

	      ret = wifi_connect_ap_inner(ssid, WIFIMG_NONE, passwd, event_label);
	  }else{
        if((key[1] == 0) && (key[2] == 0)){
            update_scan_results();
            get_key_mgmt(ssid, key);
            if((key[1] == 0) && (key[2] == 0)){
                ret = -1;
                event_code = WIFIMG_NETWORK_NOT_EXIST;
                goto end;
            }
        }

        /* ensure disconnected */
        state = get_wifi_machine_state();
        if(state == CONNECTED_STATE){
            aw_wifi_disconnect_ap(0x7fffffff);
        }
        
        /* wpa-psk */
        if(key[1] == 1){
            /* try WPA-PSK */
	          ret = wifi_connect_ap_inner(ssid, WIFIMG_WPA_PSK, passwd, event_label);
            if(ret == 0){
                return ret;
            }
        }
	  		
	  		/* wep */
	  		if(key[2] == 1){
	  		    /* try WEP */
	  		    ret = wifi_connect_ap_inner(ssid, WIFIMG_WEP, passwd, event_label);
	  		}
	  }

end:
    if(ret != 0){
        call_event_callback_function(event_code, NULL, event_label);
    }
    return ret;
}

/* cancel saved AP in wpa_supplicant.conf */
static int aw_wifi_remove_network(char *ssid, tKEY_MGMT key_mgmt)
{
    int ret = -1, len = 0;
    char cmd[CMD_LEN+1] = {0};
    char reply[REPLY_BUF_SIZE] = {0};
    char net_id[NET_ID_LEN+1] = {0};

    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return -1;
    }

    if(!ssid || !ssid[0]){
        printf("Error: ssid is null!\n");
        return -1;
    }
    
    /* check AP is exist in wpa_supplicant.conf */
    len = NET_ID_LEN+1;
    ret = wpa_conf_ssid2netid(ssid, key_mgmt, net_id, &len);
    if(ret <= 0){
        printf("Warning: %s not in wpa_supplicant.conf!\n", ssid);
        return 0;
    }
    
    /* cancel saved in wpa_supplicant.conf */
    sprintf(cmd, "REMOVE_NETWORK %s", net_id);
    ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do remove network %s error!\n", net_id);
        return -1;
    }
    
    /* save config */
	  sprintf(cmd, "%s", "SAVE_CONFIG");
	  ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do save config error!\n");
        return -1;
    }
    
    return 0;
}

static int aw_wifi_remove_all_networks()
{
    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return -1;
    }
    	
    wpa_conf_remove_all_networks();
}

static int aw_wifi_connect_ap_auto(int event_label)
{
    int i=0, ret = -1, len = 0;
    char cmd[CMD_LEN+1] = {0}, reply[REPLY_BUF_SIZE] = {0};
    char netid[NET_ID_LEN+1]={0};
    tWIFI_MACHINE_STATE wifi_machine_state;

    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return -1;
    }

    wifi_machine_state = get_wifi_machine_state();
	  if(wifi_machine_state != CONNECTED_STATE
	  	  && wifi_machine_state != DISCONNECTED_STATE){
        ret = -1;
        event_code = WIFIMG_DEV_BUSING_EVENT;
        goto end;
    }

    /* check network exist in wpa_supplicant.conf */
    if(wpa_conf_network_info_exist() == 0){
        ret = -1;
        event_code = WIFIMG_NO_NETWORK_CONNECTING;
        goto end;
    }

    netid_connecting[0] = '\0';
    disconnecting = 0;
    connecting_ap_event_label = event_label;

    /* reconnected */
	  sprintf(cmd, "%s", "RECONNECT");
    ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do reconnect error!\n");
        ret = -1;
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
    }
    
end:
    if(ret != 0){
    	  call_event_callback_function(event_code, NULL, event_label);
    }
    return ret;
}

static int aw_wifi_disconnect_ap(int event_label)
{
    int i=0, ret = -1, len = 0;
    char cmd[CMD_LEN+1] = {0}, reply[REPLY_BUF_SIZE] = {0};
    char netid[NET_ID_LEN+1]={0};
    tWIFI_MACHINE_STATE wifi_machine_state;
    
    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return -1;
    }
    
    disconnecting = 1;
    
    /* check wifi state machine */
    wifi_machine_state = get_wifi_machine_state();
    if(wifi_machine_state == CONNECTING_STATE
        || wifi_machine_state == DISCONNECTING_STATE){
        ret = -1;
        event_code = WIFIMG_DEV_BUSING_EVENT;
        goto end;
    }

    if(wifi_machine_state != L2CONNECTED_STATE
        && wifi_machine_state != CONNECTED_STATE){
        ret = -1;
        event_code = WIFIMG_OPT_NO_USE_EVENT;
        goto end;
    }

    len = NET_ID_LEN+1;
    ret = wpa_conf_get_netid_connected(netid, &len);
    if(ret <= 0){
        printf("This no connected AP!\n");
        ret = -1;
        event_code = WIFIMG_OPT_NO_USE_EVENT;
        goto end;
    }
    
    /* set disconnecting */
    set_wifi_machine_state(DISCONNECTING_STATE);
    
    /* set disconnect event label */
    disconnect_ap_event_label = event_label;
    
    /* disconnected */
	  sprintf(cmd, "%s", "DISCONNECT");
	  ret = wifi_command(cmd, reply, sizeof(reply));
    if(ret){
        printf("do disconnect network error!\n");
        ret = -1;
        event_code = WIFIMG_CMD_OR_PARAMS_ERROR;
        goto end;
    }
    
    i=0;
    do{
        usleep(200000);
        if(get_wifi_machine_state() == DISCONNECTED_STATE){
            break;
        }
        i++;
    }while(i<15);

end:
    if(ret != 0){
    	  call_event_callback_function(event_code, NULL, event_label);
    }
    return ret;
}

static const aw_wifi_interface_t aw_wifi_interface = {
    aw_wifi_add_event_callback,
    aw_wifi_is_ap_connected,
    aw_wifi_scan,
    aw_wifi_get_scan_results,
    aw_wifi_connect_ap,
    aw_wifi_connect_ap_key_mgmt,
    aw_wifi_connect_ap_auto,
    aw_wifi_add_network,
    aw_wifi_disconnect_ap,
    aw_wifi_remove_all_networks    
};

const aw_wifi_interface_t * aw_wifi_on(tWifi_event_callback pcb, int event_label)
{
    int i = 0, ret = -1, connected = 0, len = 3;
    char netid[4];

    if(gwifi_state != WIFIMG_WIFI_DISABLED){
        return NULL;
    }

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
        if(ret < 0){
            if(pcb != NULL){
                pcb(WIFIMG_WIFI_ON_FAILED, NULL, event_label);
            }
            return NULL;
        }
    }
    if(pcb != NULL){
        pcb(WIFIMG_WIFI_ON_SUCCESS, NULL, event_label);
    }
    
    gwifi_state = WIFIMG_WIFI_ENABLE;
    
    aw_wifi_add_event_callback(pcb);
    
    wifi_event_loop(NULL);
    
    /* check has network info in wpa_supplicant.conf */
    if(wpa_conf_network_info_exist() == 1){
        set_wifi_machine_state(CONNECTING_STATE);
    	  /* wpa_supplicant already run by other process and connected an ap */
        connected = wpa_conf_is_ap_connected(netid, &len);
        if(connected == 1){
            set_wifi_machine_state(CONNECTED_STATE);
            send_wifi_event(AP_CONNECTED, event_label);
            ret = 0;
        }else{
            connecting_ap_event_label = event_label;
            start_wifi_on_check_connect_timeout();
        }
    }else{
        set_wifi_machine_state(DISCONNECTED_STATE);
        event_code = WIFIMG_NO_NETWORK_CONNECTING;
        ret = -1;
    }
    
    start_wifi_scan_thread(NULL);

    if(ret != 0){
        call_event_callback_function(event_code, NULL, event_label);
    }
    
    return &aw_wifi_interface;
}

int aw_wifi_off(const aw_wifi_interface_t *p_wifi_interface)
{
    const aw_wifi_interface_t *p_aw_wifi_intf = &aw_wifi_interface;
    
    if(p_aw_wifi_intf != p_wifi_interface){
    	  call_event_callback_function(WIFIMG_WIFI_OFF_FAILED, NULL, 0);
        return -1;
    }
	
    if(gwifi_state == WIFIMG_WIFI_DISABLED){
        return 0;
    }
    
	  stop_wifi_scan_thread();
    wifi_close_supplicant_connection();
    wifi_stop_supplicant(0);
    call_event_callback_function(WIFIMG_WIFI_OFF_SUCCESS, NULL, 0);
    gwifi_state = WIFIMG_WIFI_DISABLED;
    return 0;
}

int aw_wifi_get_wifi_state()
{
    int ret = -1, len = 0;
    char netid[4] = {0};
    tWIFI_MACHINE_STATE machine_state;
    int tmp_state;
    
    /* wpa_supplicant already running not by self process */
    if(gwifi_state == WIFIMG_WIFI_DISABLED){
    	  /* check wifi already on by self process or other process */
        ret = wifi_connect_to_supplicant();
        if(ret){
            printf("WiFi not on\n");
            return WIFIMG_WIFI_DISABLED;
        }
        
    	  /* sync wifi state by wpa_supplicant */
    	  len = 3;
        ret = aw_wifi_is_ap_connected(netid, &len);
        if(ret == 1){
            tmp_state = WIFIMG_WIFI_CONNECTED;
        }else{
            tmp_state = WIFIMG_WIFI_DISCONNECTED;
        }
        
        /*close connect */
        wifi_close_supplicant_connection();
        return tmp_state;
    }

    machine_state = get_wifi_machine_state();
    if(machine_state == DISCONNECTED_STATE){
        gwifi_state = WIFIMG_WIFI_DISCONNECTED;
    }else if(machine_state == CONNECTING_STATE
        || machine_state == DISCONNECTING_STATE
        || machine_state == L2CONNECTED_STATE){
        gwifi_state = WIFIMG_WIFI_BUSING;
    }else if(machine_state == CONNECTED_STATE){
        gwifi_state = WIFIMG_WIFI_CONNECTED;
    }

    return gwifi_state;
}
