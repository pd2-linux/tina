#ifndef __WIFI_INTF_H
#define __WIFI_INTF_H

#if __cplusplus
extern "C" {
#endif

#define MAX_CALLBCAKS_COUNT  1024

typedef enum {
    WIFIMG_NONE = 0,
    WIFIMG_WPA_PSK,
    WIFIMG_WPA2_PSK,
    WIFIMG_WEP,
}tKEY_MGMT;

typedef enum{
    WIFIMG_WIFI_ON_SUCCESS = 0xf000,
    WIFIMG_WIFI_ON_FAILED,
    WIFIMG_WIFI_OFF_SUCCESS,
    WIFIMG_WIFI_OFF_FAILED,
    WIFIMG_NO_NETWORK_CONNECTING,
    WIFIMG_CMD_OR_PARAMS_ERROR,
    WIFIMG_KEY_MGMT_NOT_SUPPORT,
    WIFIMG_OPT_NO_USE_EVENT,
    WIFIMG_NETWORK_NOT_EXIST,
    WIFIMG_DEV_BUSING_EVENT,
    WIFIMG_NETWORK_DISCONNECTED,
    WIFIMG_NETWORK_CONNECTED,
    WIFIMG_PASSWORD_FAILED,
    WIFIMG_CONNECT_TIMEOUT,
}tWIFI_EVENT;

typedef enum{
    WIFI_MANAGER_SUCCESS = 0,
    WIFI_MANAGER_FAILED = -1,
}tRETURN_CODE;

typedef enum{
    WIFIMG_WIFI_ENABLE = 0x1000,
    WIFIMG_WIFI_DISCONNECTED,
    WIFIMG_WIFI_BUSING,
    WIFIMG_WIFI_CONNECTED,
    WIFIMG_WIFI_DISABLED,
}tWIFI_STATE;

typedef void (*tWifi_event_callback)(tWIFI_EVENT wifi_event, void *buf, int event_lebal);

typedef struct{
    int (*add_event_callback)(tWifi_event_callback pcb);
    int (*get_wifi_state)(void);
    int (*is_ap_connected)(char *ssid, int *len);
    int (*start_scan)(void);
    int (*get_scan_results)(char *result, int *len);
    int (*connect_ap)(const char *ssid, const char *passwd, int event_label);
    int (*connect_ap_key_mgmt)(const char *ssid, tKEY_MGMT key_mgnt, const char *passwd, int event_label);
    int (*connect_ap_auto)(int event_label);
    int (*add_network)(const char *ssid, tKEY_MGMT key_mgnt, const char *passwd, int event_label);
    int (*disconnect_ap)(int event_label);
    int (*remove_all_networks)(void);
}aw_wifi_interface_t;

const aw_wifi_interface_t * aw_wifi_on(tWifi_event_callback pcb, int event_label);
int aw_wifi_off(const aw_wifi_interface_t *p_wifi_interface_t);

#if __cplusplus
};  // extern "C"
#endif

#endif