#ifndef __WIFI_EVENT_H
#define __WIFI_EVENT_H

#include "wifi_intf.h"

#define EVENT_BUF_SIZE 2048

#if __cplusplus
extern "C" {
#endif

#define MAX_ASSOC_REJECT_COUNT  3
#define MAX_RETRIES_ON_AUTHENTICATION_FAILURE 2

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
int set_cur_wifi_event(tWIFI_EVENT_INNER event);
void set_scan_start_flag();
int  get_scan_status();
void reset_assoc_reject_count();
int  get_assoc_resject_count();
int add_wifi_event_callback_inner(tWifi_event_callback pcb);
int call_event_callback_function(tWIFI_EVENT wifi_event, char *buf, int event_label);
int reset_wifi_event_callback();
#if __cplusplus
};  // extern "C"
#endif

#endif /*__WIFI_EVENT_H*/
