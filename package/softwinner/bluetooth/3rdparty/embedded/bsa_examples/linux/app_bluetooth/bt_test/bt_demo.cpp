#include <bluetooth_socket.h>
#include <unistd.h>

void bt_event_f(BT_EVENT event)
{
    switch(event)
    {
    	  case BT_AVK_CONNECTED_EVT:
    	  {
    	  	  printf("Media audio connected!\n");
    	  	  break;
    	  }
    	  
    	  case BT_AVK_DISCONNECTED_EVT:
    	  {
    	  	  printf("Media audio disconnected");
    	  	  break;
    	  }
    	  
    	  case BT_AVK_START_EVT:
    	  {
    	  	  printf("Media start playing!\n");
    	  	  break;
    	  }
    	  
    	  case BT_AVK_STOP_EVT:
    	  {
    	      printf("Media stop playing!\n");
    	      break;	
    	  }
    	  
    	  default:
    	      break;
    }	
}


int main(){
    c_bt c;
    usleep(2000*1000);
    c.do_test();
    c.set_bt_name("aw bt test001");
    c.set_callback(bt_event_f);
 
    while(1);
}