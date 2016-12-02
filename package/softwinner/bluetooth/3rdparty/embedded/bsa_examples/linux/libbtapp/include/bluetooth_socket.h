#ifndef __BLUETOOTH_SOCKET_H__
#define __BLUETOOTH_SOCKET_H__

#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <unistd.h>

#define BT_NAME_PATH_LEN   256

#ifndef BT_ADDR_LEN
#define BT_ADDR_LEN     6
typedef unsigned char BT_ADDR[BT_ADDR_LEN];
#endif

#define BT_AVK_ENABLE 	1
#define BT_HS_ENABLE	1

enum BT_EVENT{
    BT_AVK_CONNECTED_EVT = 0,
    BT_AVK_DISCONNECTED_EVT,
    BT_AVK_START_EVT,             /* stream data transfer started */
    BT_AVK_STOP_EVT,	          /* stream data transfer stopped */
    BT_AVK_CONNECT_COMPLETED_EVT,
    BT_HS_CONNECTED_EVT = 0xf0,
    BT_HS_DISCONNECTED_EVT,
    BT_HS_RING_EVT,
    BT_HS_OK_EVT,
    BT_DISCOVER_COMPLETE,
    BT_HS_ERROR_EVT
};

typedef void (tBtCallback)(BT_EVENT event, void *reply, int *len);

#ifndef AVK_MUSIC_INFO_LEN_MAX
#define AVK_MUSIC_INFO_LEN_MAX 102
#endif

typedef struct{
    unsigned char title[AVK_MUSIC_INFO_LEN_MAX];
    unsigned char artist[AVK_MUSIC_INFO_LEN_MAX];
    unsigned char album[AVK_MUSIC_INFO_LEN_MAX];
}tBT_AVK_MUSIC_INFO;

class c_bt
{
public:
    c_bt();
    virtual ~c_bt();
private:
    char   bt_wd[256];
    tBtCallback *pBtCb;
    int    bt_on_status;

public:
    int bt_on(char *bt_addr);
    int bt_on_no_avrcp(char *bt_addr);
    int bt_off();
    int set_bt_name(const char *bt_name); // strlen(bt_name) <= MAX_DATA_T_LEN-1
    int set_dev_discoverable(int enable);
    int set_dev_connectable(int enable);
    int start_discovery(int time);
    int get_disc_results(char *disc_results, int *len);
    int connect_auto();
    int connect_dev_by_addr(BT_ADDR bt_addr);
    int disconnect();
    int reset_avk_status();
    int avk_play();
    int avk_pause();
    int avk_stop();
    int avk_close_pcm_alsa();
    int avk_resume_pcm_alsa();
    int avk_previous();
    int avk_next();
    int avk_get_music_info(tBT_AVK_MUSIC_INFO *p_avk_music_info);
    int hs_pick_up();
    int hs_hung_up();
    void set_callback(tBtCallback *pCb);
    void event_callback(BT_EVENT bt_event, void *reply, int *len);
    void do_test();
};

#endif /* __BLUETOOTH_CLIENT_H__ */
