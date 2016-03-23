#ifndef __WIFI_INTF_H
#define __WIFI_INTF_H

#if __cplusplus
extern "C" {
#endif

typedef enum {
    NONE = 0,
    WPA_PSK,
    WPA_EAP,
    WEP,
    WPA2_PSK,
}tKEY_MGMT;

typedef enum{
    NETWORK_CONNECTED = 0,
    NETWORK_DISCONNECTED,
    PASSWORD_FAILED,
    CONNECT_TIMEOUT,    	
}tWIFI_STATE;

typedef void (*tWifi_event_callback)(tWIFI_STATE wifi_state, void *buf);

int wifi_on(tWifi_event_callback pcb /* =NULL */);
int wifi_off();
int is_ap_connected(char *ssid, int *len);
int scan();
int get_scan_results(char *result, int *len);
int connect_ap(const char *ssid, const char *passwd);
int connect_ap_key_mgmt(const char *ssid, tKEY_MGMT key_mgnt, const char *passwd);
int disconnect_ap();
int connect_ap_auto();
int remove_all_networks();

#if __cplusplus
};  // extern "C"
#endif

#endif