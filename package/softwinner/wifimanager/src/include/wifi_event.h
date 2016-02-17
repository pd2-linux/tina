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

void wifi_event_loop(tWifi_event_callback pcb);

#if __cplusplus
};  // extern "C"
#endif

#endif /*__WIFI_EVENT_H*/