#ifndef __WIFI_STATE_H
#define __WIFI_STATE_H

#if __cplusplus
extern "C" {
#endif

typedef enum{
    DISCONNECTED_STATE = 0xf0,
    CONNECTING_STATE,
    L2CONNECTED_STATE,
    CONNECTED_STATE,
    DISCONNECTING_STATE,
}tWIFI_MACHINE_STATE;

int set_wifi_machine_state(tWIFI_MACHINE_STATE state);
tWIFI_MACHINE_STATE get_wifi_machine_state();

#if __cplusplus
};  // extern "C"
#endif

#endif /*__WIFI_STATE_H*/