#ifndef __BLUETOOTH_INTERFACE_H__
#define __BLUETOOTH_INTERFACE_H__



/* define bt app event */
typedef enum{
    APP_AVK_CONNECTED_EVT = 0,
    APP_AVK_DISCONNECTED_EVT = 1,
    APP_AVK_START_EVT = 2,             /* stream data transfer started */
    APP_AVK_STOP_EVT = 3,	       /* stream data transfer stopped */
    APP_AVK_CONNECT_COMPLETED_EVT = 4,
    APP_AUDIO_OPEN_REQ_EVT = 5,
    APP_HS_CONNECTED_EVT = 6,
    APP_HS_DISCONNECTED_EVT = 7,
    APP_HS_AUDIO_OPEN_EVT = 8,
    APP_HS_AUDIO_CLOSE_EVT = 9,
    APP_MGR_DISC_RESULTS = 10,
}APP_BT_EVENT;

int bluetooth_start(void *p, char *p_conf);
void bluetooth_stop();
void start_app_avk();
void stop_app_avk();
void s_set_bt_name(const char *name);
void s_set_discoverable(int enable);
void s_set_connectable(int enable);
void s_start_discovery(int time);
int  s_get_disc_results(char *results, int *len);
void s_set_volume(int volume);
void s_set_volume_up();
void s_set_volume_down();
int  s_connect_auto();
void s_disconnect();
void s_avk_play();
void s_avk_pause();
void s_avk_stop();
void s_avk_play_previous();
void s_avk_play_next();
void s_hs_pick_up();
void s_hs_hung_up();

#endif /* __BLUETOOTH_INTERFACE_H__ */
