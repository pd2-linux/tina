#ifndef __WIFI_EVENT_H
#define __WIFI_EVENT_H

#include "wifi_intf.h"

#define EVENT_BUF_SIZE 2048

#if __cplusplus
extern "C" {
#endif

enum WPA_EVENT{
    CONNECTED    = 1,
    DISCONNECTED,
    STATE_CHANGE,
    SCAN_RESULTS,
    LINK_SPEED,
    TERMINATING,
    DRIVER_STATE,
    EAP_FAILURE,
    ASSOC_REJECT,
    UNKNOWN,
};

typedef enum{
    AP_DISCONNECTED,
    AP_CONNECTED,
    PASSWORD_INCORRECT,
    CONNECT_AP_TIMEOUT,
    OBTAINING_IP_TIMEOUT,
}tWIFI_EVENT_INNER;

void wifi_event_loop(tWifi_event_callback pcb);
void start_wifi_on_check_connect_timeout();
void start_check_connect_timeout();
tWIFI_EVENT_INNER  get_cur_wifi_event();
void send_wifi_event(tWIFI_EVENT_INNER event_inner, int event_label);
void set_scan_start_flag();
int  get_scan_status();
int add_wifi_event_callback_inner(tWifi_event_callback pcb);
int call_event_callback_function(tWIFI_EVENT wifi_event, char *buf, int event_label);

#if __cplusplus
};  // extern "C"
#endif

#endif /*__WIFI_EVENT_H*/