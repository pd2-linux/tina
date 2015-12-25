#ifndef __BLUETOOTH_INTERFACE_H__
#define __BLUETOOTH_INTERFACE_H__

//#include "bluetooth_socket.h"

/* define bt app event */
enum APP_BT_EVENT{
	  APP_AVK_CONNECTED_EVT = 0, 
	  APP_AVK_DISCONNECTED_EVT = 1, 
    APP_AVK_START_EVT = 2,       /* stream data transfer started */
    APP_AVK_STOP_EVT = 3,	       /* stream data transfer stopped */
    APP_AUDIO_OPEN_REQ_EVT = 4,
    APP_HS_CONNECTED_EVT = 5,
    APP_HS_DISCONNECTED_EVT = 6,
    APP_HS_AUDIO_OPEN_EVT = 7,
    APP_HS_AUDIO_CLOSE_EVT =8,
};

int bluetooth_start(void *p, char *p_conf);
void s_avk_play();
void s_avk_pause();
void s_avk_play_previous();
void s_avk_play_next();
void s_hs_pick_up();
void s_hs_hung_up();

#endif /* __BLUETOOTH_INTERFACE_H__ */