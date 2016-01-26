#include <bluetooth_socket.h>
#include <unistd.h>

static int status = 0;
c_bt c;

void bt_event_f(BT_EVENT event)
{
    switch(event)
    {
    	  case BT_AVK_CONNECTED_EVT:
    	  {
    	  	  printf("Media audio connected!\n");
    	  	  c.set_dev_discoverable(0);
    	  	  c.set_dev_connectable(0);
    	  	  break;
    	  }
    	  
    	  case BT_AVK_DISCONNECTED_EVT:
    	  {
    	  	  printf("Media audio disconnected");
    	  	  c.set_dev_connectable(1);
    	  	  c.set_dev_discoverable(1);
    	  	  break;
    	  }
    	  
    	  case BT_AVK_START_EVT:
    	  {
    	  	  printf("Media start playing!\n");
    	  	  status = 1;
    	  	  break;
    	  }
    	  
    	  case BT_AVK_STOP_EVT:
    	  {
    	      printf("Media stop playing!\n");
    	      status = 0;
    	      break;	
    	  }
    	  
    	  default:
    	      break;
    }	
}


int main(int argc, char *args[]){
    
    if(argc >= 2){
        c.set_bt_wd(args[1]);
    }
    
    if(argc >= 3){
       c.bt_on(args[2]);
    } else {
       c.bt_on(NULL); 
    }
      
    usleep(2000*1000);
    c.do_test();
    
/*    
    c.bt_off();
    
    c.bt_on(NULL);
    printf("bt do second test!\n");
    c.do_test();
    c.set_bt_name("aw bt test001");
    c.set_callback(bt_event_f);
 
 
    while(1){
        usleep(2000*1000);
        if(status == 1){
            usleep(5000*1000);
            c.avk_pause();
            usleep(5000*1000);
            c.avk_play();
            
            break;
        }    
    }
*/
 
    while(1);
}