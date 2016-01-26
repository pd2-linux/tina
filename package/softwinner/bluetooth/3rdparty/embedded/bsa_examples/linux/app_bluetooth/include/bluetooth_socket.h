#ifndef __BLUETOOTH_SOCKET_H__
#define __BLUETOOTH_SOCKET_H__

#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <unistd.h>
#include <interface.h>

#define BT_NAME_PATH_LEN   256

enum {
    BT_CMD_SET_NAME = 0xf0,
    BT_CMD_SET_DISCOVERABLE,
    BT_CMD_SET_CONNECTABLE,
    BT_CMD_PLAY,
    BT_CMD_PAUSE,
    BT_CMD_PRE,
    BT_CMD_NEXT,
    BT_CMD_PICK_UP,
    BT_CMD_HUNG_UP,
    BT_CMD_TRANSACT_EVENT,
    
    BT_CMD_DO_TEST,
};

enum BT_EVENT{
	  BT_AVK_CONNECTED_EVT = 0, 
	  BT_AVK_DISCONNECTED_EVT, 
    BT_AVK_START_EVT,        /* stream data transfer started */
    BT_AVK_STOP_EVT,	       /* stream data transfer stopped */
    BT_HS_CONNECTED_EVT = 0xf0,
    BT_HS_DISCONNECTED_EVT,
    BT_HS_RING_EVT,
    BT_HS_OK_EVT,
    BT_HS_ERROR_EVT
};


typedef struct bt_data{
    int data1;
    int data2;
}bt_data;

typedef void (tBtCallback)(BT_EVENT event);

class c_bt : public c_interface
{
public:
    c_bt();
    virtual ~c_bt();
private:
    char   bt_wd[256];
    tBtCallback *pBtCb;

protected:
    void onTransact(request_t* request,data_t* data);

public:
	  int set_bt_wd(char *pwd);
	  int bt_on(char *bt_addr);
	  int bt_off();
    int set_bt_name(const char *bt_name); // strlen(bt_name) <= MAX_DATA_T_LEN-1
    int set_dev_discoverable(int enable);
    int set_dev_connectable(int enable);   
    int avk_play();
    int avk_pause();
    int avk_previous();
    int avk_next();
    int hs_pick_up();
    int hs_hung_up();
    void set_callback(tBtCallback *pCb);
    void do_test();
};

class s_bt : public s_interface
{
public:
    s_bt();
    virtual ~s_bt();

protected:
    void onTransact(request_t* request,data_t* data);

public:
    int do_transact_event(BT_EVENT event);
};


#endif /* __BLUETOOTH_CLIENT_H__ */