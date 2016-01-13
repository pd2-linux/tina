#include "bluetooth_socket.h"

extern "C"
{
#include "bluetooth_interface.h" 	
}

//extern "C" void s_avk_play();
//extern "C" void s_avk_pause();
//extern "C" void s_avk_play_previous();
//extern "C" void s_avk_play_next();
//extern "C" void s_hs_pick_up();
//extern "C" void s_hs_hung_up();

const char* UNIX_DOMAIN = "tina-bluetooth-service";

c_bt::c_bt(){
    setSocketName(UNIX_DOMAIN);
    init();
}

c_bt::~c_bt(){

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
            printf("BT EVENT %d comming\n", rec_event);
            if(pBtCb != NULL)
            {
                pBtCb(rec_event);    	
            }
        }
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
    
    memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_SET_NAME;
    
    memset(&senddata, 0, sizeof(senddata));
    memcpy(senddata.buf,(void *)name, MAX_DATA_T_LEN - 1);
    senddata.buf[MAX_DATA_T_LEN - 1] = '\0';
    senddata.len = sizeof(senddata.buf);
    
	  transact(&request, &senddata);
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
    
    printf("This is test socket!\n");
    printf("Client T->BT\n");
    
    memset(&request, 0 ,sizeof(request));
    request.code = BT_CMD_DO_TEST;
    
    memset(&data,0,sizeof(data));
    memset(&senddata, 0, sizeof(senddata));
    data.data1 = 'B';
    data.data2 = 'T';
    senddata.len = sizeof(bt_data);
    memcpy(senddata.buf,(void*)&data,senddata.len);
    
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
    	  char bt_name[MAX_DATA_T_LEN] = {0};
        if (data) {
            strncpy(bt_name, data->buf, MAX_DATA_T_LEN-1);
            bt_name[MAX_DATA_T_LEN - 1] = '\0';  
        }
    	  printf("set bt name %s\n", bt_name);
    	  s_set_bt_name(bt_name);
    	  break;
    }
    
    case BT_CMD_PLAY:
    {
        printf("s play comming\n");
        s_avk_play();
        break;
    }
    
    case BT_CMD_PAUSE:
    {
        printf("s pause comming\n");
        s_avk_pause();
        break;
    }
    
    case BT_CMD_PRE:
    {
        printf("s previous comming\n");
        s_avk_play_previous();
        break;
    }
    
    case BT_CMD_NEXT:
    {
        printf("s next comming\n");
        s_avk_play_next();
        break;
    }
    
    case BT_CMD_PICK_UP:
    {
    	  s_hs_pick_up();
    	  printf("s pick up cmd comming\n");
    	  break;
    }
    
    case BT_CMD_HUNG_UP:
    {
    	  s_hs_hung_up();
    	  printf("s hung up cmd comming\n");
    	  break;
    }
        
    case TRANSACT_CODE_CLIENT_CONNECT:
    {
        printf("client connected\n");
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
    	  printf("server received test\n");
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
    senddata.len = sizeof(bt_data);
    memcpy(senddata.buf,(void*)&data,senddata.len);
	  
	  transact(&request,&senddata);
}

extern "C" void bt_event_transact(s_bt *p, APP_BT_EVENT event)
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
        
        default:
        	;		
    }
}