#ifndef __NETWORK_MANAGER_H
#define __NETWORK_MANAGER_H

#if __cplusplus
extern "C" {
#endif

#define SCAN_BUF_LEN      4096
#define KEY_NONE_INDEX    0
#define KEY_WPA_PSK_INDEX 1
#define KEY_WEP_INDEX     2

void start_wifi_scan_thread(void *args);
void stop_wifi_scan_thread();
void pause_wifi_scan_thread();
void resume_wifi_scan_thread();
int  update_scan_results();
int  get_scan_results_inner(char *results, int *len);
int get_key_mgmt(const char *ssid, int key_mgmt_info[]);

#if __cplusplus
};  // extern "C"
#endif

#endif /* __NETWORK_MANAGER_H */


