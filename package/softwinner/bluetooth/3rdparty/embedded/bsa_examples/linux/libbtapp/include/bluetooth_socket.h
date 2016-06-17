#ifndef __BLUETOOTH_SOCKET_H__
#define __BLUETOOTH_SOCKET_H__

#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <unistd.h>

#define BT_NAME_PATH_LEN   256

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

typedef void (tBtCallback)(BT_EVENT event);

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
    int bt_off();
    int set_bt_name(const char *bt_name); // strlen(bt_name) <= MAX_DATA_T_LEN-1
    int set_dev_discoverable(int enable);
    int set_dev_connectable(int enable);
    int start_discovery(int time);
    int get_disc_results(char *disc_results, int *len);
    int connect_auto();
    int disconnect();
    int reset_avk_status();
    int avk_play();
    int avk_pause();
    int avk_previous();
    int avk_next();
    int hs_pick_up();
    int hs_hung_up();
    void set_callback(tBtCallback *pCb);
    void event_callback(BT_EVENT bt_event);
    void do_test();
};

#endif /* __BLUETOOTH_CLIENT_H__ */
