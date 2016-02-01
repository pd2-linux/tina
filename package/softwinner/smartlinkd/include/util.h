#ifndef __UTIL_H__
#define __UTIL_H__

#define LEN 65
#define KEY_LEN 16

#define IFNAME "wlan0"
#define CHECK_WIFI_SHELL "lsmod | grep bcmdhd"
#define CHECK_WLAN_SHELL "ifconfig | grep "IFNAME
#define Broadcom_Setup "/setup"
#define Realtek_Setup "/arikiss"

struct ap_info{
	char ssid[LEN];
	char password[LEN];
	int security;
};
struct sender_info{
	char ip[16];/* ipv4 max length */
	int port;
};
struct _info{
	struct ap_info base_info;
	int protocol;
	int airkiss_random;
	struct sender_info ip_info;
};

/* wifi modules */
enum {
	BROADCOM = 0,
	REALTEK,
	ALLWINNERTCH,
};

/* wlan security */
enum {
	SECURITY_NONE = 0,
	SECURITY_WEP,
	SECURITY_WPA,
	SECURITY_WPA2,
};

/* protocols */
enum {
	PROTO_COOEE = 0,
	PROTO_NEEZE,
	PROTO_AKISS,
	PROTO_XIAOMI,
	PROTO_CHANGHONG,
	PROTO_MAX,
	PROTO_FAIL = 0xff,
};

//cmd for processes
enum {
	START = 0,
	FINISHED,
	CONFIG_NETWORK,  //useless now
	ENABLE_NETWORK,  //useless now
	FAILED,
};

struct _cmd{
	int cmd;
	char usekey;
	char AESKey[KEY_LEN];
	struct _info info;
};
#endif /* __UTIL_H__ */