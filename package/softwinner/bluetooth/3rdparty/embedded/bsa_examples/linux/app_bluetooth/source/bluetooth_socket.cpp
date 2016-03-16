#include "bluetooth_socket.h"
#include <stdlib.h>
#include <unistd.h>

extern "C"
{
#include "bluetooth_interface.h" 	
}

const char* UNIX_DOMAIN = "tina-bluetooth-service";

c_bt::c_bt(){
    bt_wd[0] = '.';
    bt_wd[1] = '\0';
    disc_flag = 0;
}

c_bt::~c_bt(){

}

int c_bt::set_bt_wd(char *pwd){
    if(pwd && pwd[0]){
        strncpy(bt_wd, pwd, 256);
    }
}

int c_bt::bt_on(char *bt_addr){
	
    char cmd[512] = {0};	
	  setSocketName(UNIX_DOMAIN);
    if (init() != 0) {
        /* start bt server */
        if(!bt_addr || !bt_addr[0]){
        	  snprintf(cmd, 512, "/etc/bluetooth/btenable.sh on %s", bt_wd);
        } else {
            snprintf(cmd, 512, "/etc/bluetooth/btenable.sh on %s %s", bt_wd, bt_addr);
        }
        
        system(cmd);
        usleep(1000000);
        init();
    }
    return 0;
}

int c_bt::bt_off(){
    /* stop bt server */
    system("/etc/bluetooth/btenable.sh off");
    return 0;
}

void c_bt::onTransact(request_t* request,data_t* data){
    if(!request)
    {
    	  printf("Error: s_bt received request is NULL!\n");
        return ;	
    }
     
    switch(request->code){
    case BT_CMD_TRANSACT_EVENT:
    {
    	  if(data->len == sizeof(bt_data))
    	  {
            bt_data* d = (bt_data*)data->buf;
            BT_EVENT rec_event = (BT_EVENT)(d->data1);
            if(pBtCb != NULL)
            {
                pBtCb(rec_event);    	
            }
        }
        break;
    }
    
    case BT_CMD_TRANSACT_DISC_RESULTS:
    {
    	  char *buf, *ptr;
    	  int len = 0;
    	  
    	  buf = (char *)data->buf;
    	  len = data->len;
    	  
    	  if(len == 0){
    	      disc_flag = 1;
    	      dev_info[0] = '\0';
    	      break;
    	  }
    	  
    	  /* parse disc result */
    	  strncpy(dev_info, buf, 4095);
    	  dev_info[4095] = '\0';
    	  
    	  ptr = strstr(dev_info, "Dev:");
    	  while(ptr != NULL){
    	      dev_nums++;
    	      buf=strchr(ptr, '\n');
    	      if(buf != NULL){
    	          buf++;
    	          ptr = strstr(buf, "Dev:");    
    	      }else{
    	          ptr = NULL;
    	      }
    	  }
    	  
    	  disc_flag = 1;
    	  break;
    }
    
    
    default:
        printf("s_bt code: %x\n",request->code);

    }
}

int c_bt::set_bt_name(const char *name)
{
    request_t  request;
    data_t senddata;
    char in_name[BT_NAME_PATH_LEN];
    
    memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_SET_NAME;
    
    memset(&senddata, 0, sizeof(senddata));
    if (!name || !name[0]){
        printf("Set bt name NULL! Use default!\n");
        return 0;    	
    } else {
        strncpy(in_name, name, BT_NAME_PATH_LEN);
        in_name[BT_NAME_PATH_LEN-1] = '\0';
        senddata.buf = in_name;
        senddata.len = strlen(in_name) + 1;
    }
    
	  transact(&request, &senddata);
    return 0;	
}

int c_bt::set_dev_discoverable(int enable)
{
    request_t  request;
    bt_data    data;
    data_t     senddata;
    
    memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_SET_DISCOVERABLE;
    
    memset(&senddata, 0, sizeof(senddata));
    memset(&data, 0, sizeof(data));
    if(enable != 0){
        data.data1 = 1;    
    } else {
        data.data1 = 0;
    }
    
    senddata.buf = (void *)&data;
    senddata.len = sizeof(data);
    
	  transact(&request, &senddata);
    return 0;    	
}

int c_bt::set_dev_connectable(int enable)
{
    request_t  request;
    bt_data    data;
    data_t     senddata;
    
    memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_SET_CONNECTABLE;
    
    memset(&senddata, 0, sizeof(senddata));
    memset(&data, 0, sizeof(data));
    if(enable != 0){
        data.data1 = 1;    
    } else {
        data.data1 = 0;
    }
    senddata.buf = (void *)&data;
    senddata.len = sizeof(data);
    
	  transact(&request, &senddata);
    return 0;    	
}

int c_bt::start_discovery(int time)
{
    request_t  request;
    bt_data    data;
    data_t     senddata;
    
    memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_START_DISCOVERY;
    
    memset(&senddata, 0, sizeof(senddata));
    memset(&data, 0, sizeof(data));
    data.data1 = time;   
    senddata.buf = (void *)&data;
    senddata.len = sizeof(data);
	  transact(&request, &senddata);
	  
	  dev_nums = 0;
	  disc_flag = 0;
    return 0;	
}

int c_bt::get_disc_results(char *disc_results, int *len)
{  
	  int times = 0;
	  
	  while((disc_flag == 0) && (times < 300)){
	      usleep(100*1000);
	      times++;
	  }
	  
	  if(disc_flag == 0 || dev_nums == 0){
	      disc_results[0] = '\0';
	      *len = 0;
	      return 0;
	  }else{
	      strncpy(disc_results, dev_info, *len);
	      *len = strlen(dev_info);
	      disc_results[*len] = '\0';
	      return dev_nums;
	  }
}

int c_bt::connect_auto()
{
    request_t  request;
	  
	  memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_CONNECT_AUTO;
    
	  transact(&request,NULL);
    return 0;	
}

int c_bt::disconnect()
{
    request_t  request;
	  
	  memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_DISCONNECT;
    
	  transact(&request,NULL);
    return 0;	
}

int c_bt::avk_play()
{
	  request_t  request;
	  
	  memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_PLAY;
    
	  transact(&request,NULL);
    return 0;
}

int c_bt::avk_pause()
{
	  request_t  request;
	  
	  memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_PAUSE;
    
	  transact(&request,NULL);
    return 0;
}

int c_bt::avk_previous()
{
	  request_t  request;
	  
	  memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_PRE;
    
	  transact(&request,NULL);
    return 0;	
}

int c_bt::avk_next()
{
    request_t  request;
	  
	  memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_NEXT;
    
	  transact(&request,NULL);
	  return 0;
}

int c_bt::hs_pick_up()
{
	  request_t  request;
	  
	  memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_PICK_UP;
    
	  transact(&request,NULL);
	  return 0;
}

int c_bt::hs_hung_up()
{
	  request_t  request;
	  
	  memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_HUNG_UP;
    
	  transact(&request,NULL);
    return 0;
}

void c_bt::set_callback(tBtCallback *pCb)
{
	  pBtCb = pCb;
}

void c_bt::do_test()
{
	  request_t  request;
	  bt_data data;
    data_t senddata;
    
    memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_DO_TEST;
    
    memset(&data,0,sizeof(data));
    memset(&senddata, 0, sizeof(senddata));
    data.data1 = 'B';
    data.data2 = 'T';
    senddata.buf = (void *)&data;
    senddata.len = sizeof(data);
    
    transact(&request, &senddata);	
}

//////////////////////////////////////////////
/////////////////BT sever ////////////////////
s_bt::s_bt(){
    setSocketName(UNIX_DOMAIN);
    init();
}

s_bt::~s_bt(){

}

void s_bt::onTransact(request_t* request,data_t* data){
    if(!request)
    {
    	  printf("Error: s_bt received request is NULL!\n");
        return ;	
    }
    
    switch(request->code){
    case BT_CMD_SET_NAME:
    {
    	  char bt_name[BT_NAME_PATH_LEN] = {0};
        if (data && data->buf) {
            memcpy(bt_name, data->buf, BT_NAME_PATH_LEN - 1);
            bt_name[BT_NAME_PATH_LEN - 1] = '\0';  
        }
    	  s_set_bt_name(bt_name);
    	  break;
    }
    
    case BT_CMD_SET_DISCOVERABLE:
    {
        bt_data  *s_data;
        if (data && data->buf){
            s_data = (bt_data *)data->buf;
        
            if(s_data->data1 != 0){
                s_set_discoverable(1);
            } else {
                s_set_discoverable(0);      
            }
        }
        break;        	
    }
    
    case BT_CMD_SET_CONNECTABLE:
    {
        bt_data  *s_data;
        
        if (data && data->buf){
            s_data = (bt_data *)data->buf;
            
            if(s_data->data1 != 0){
                s_set_connectable(1);
            } else {
                s_set_connectable(0);      
            }
        }
        break;        	
    }
    
    case BT_CMD_START_DISCOVERY:
    {
        bt_data  *s_data;
        int time;
        
        if (data && data->buf){
            s_data = (bt_data *)data->buf;
            
            time = s_data->data1;
            s_start_discovery(time);
        }
        break;	
    }
    
    case BT_CMD_CONNECT_AUTO:
    {
        s_connect_auto();
        break;	
    }
    
    case BT_CMD_DISCONNECT:
    {
        s_disconnect();
        break;	
    }
    
    case BT_CMD_PLAY:
    {
        s_avk_play();
        break;
    }
    
    case BT_CMD_PAUSE:
    {
        s_avk_pause();
        break;
    }
    
    case BT_CMD_PRE:
    {
        s_avk_play_previous();
        break;
    }
    
    case BT_CMD_NEXT:
    {
        s_avk_play_next();
        break;
    }
    
    case BT_CMD_PICK_UP:
    {
    	  s_hs_pick_up();
    	  break;
    }
    
    case BT_CMD_HUNG_UP:
    {
    	  s_hs_hung_up();
    	  break;
    }
        
    case TRANSACT_CODE_CLIENT_CONNECT:
    {
        m_clientfd = request->client_handle;
        break;
    }
    
    case TRANSACT_CODE_CLIENT_DISCONNECT:
    {
        printf("client disconnected\n");
        m_clientfd = -1;
        break;
    }

    case BT_CMD_DO_TEST:
    {
    	  if(data->len == sizeof(bt_data))
    	  {
    	      bt_data* d = (bt_data*)data->buf;
            int d1 = d->data1;
            int d2 = d->data2;
            if((d1 == 'B') && (d2 == 'T'))
            {
                printf("Test OK!\n");
            }
            else
            {
            	  printf("Test Failed!\n");
            }
    	  }
    	  break;
    }

    default:
        printf("s_bt code: %x\n",request->code);

    }
}

int s_bt::do_transact_event(BT_EVENT event)
{
	  request_t  request;
	  bt_data data;
    data_t senddata;
    
    printf("do transact event %d\n", event);
    
    memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_TRANSACT_EVENT;
    request.client_handle = m_clientfd;
    
    memset(&data,0,sizeof(data));
    memset(&senddata, 0, sizeof(senddata));
    data.data1 = event;
    data.data2 = 0;
    senddata.buf = (void *)&data;
    senddata.len = sizeof(data);
	  
	  transact(&request,&senddata);
}

int s_bt::do_transact_disc_results(char *reply, int *len)
{
	  request_t  request;
    data_t senddata;
    
    memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_TRANSACT_DISC_RESULTS;
    request.client_handle = m_clientfd;
    
    memset(&senddata, 0, sizeof(senddata));
    senddata.buf = (void *)reply;
    senddata.len = *len;
	  
	  transact(&request,&senddata);
}

extern "C" void bt_event_transact(s_bt *p, APP_BT_EVENT event, char *reply, int *len)
{
    switch(event)
    {
    	  case APP_AVK_CONNECTED_EVT:
    	  {
    	      p->do_transact_event(BT_AVK_CONNECTED_EVT);
    	      break;	
    	  }
    	  
    	  case APP_AVK_DISCONNECTED_EVT:
    	  {
    	  	  p->do_transact_event(BT_AVK_DISCONNECTED_EVT);
    	  	  break;
    	  }
    	  
    	  
        case APP_AVK_START_EVT:
        {
            p->do_transact_event(BT_AVK_START_EVT);	  
            break;	 
        }   	
        	
        case APP_AVK_STOP_EVT:
        {
            p->do_transact_event(BT_AVK_STOP_EVT);
            break;
        }
        
        case APP_MGR_DISC_RESULTS:
        {
        	  p->do_transact_disc_results(reply, len);
            break;	
        }
        
        default:
        	;		
    }
}