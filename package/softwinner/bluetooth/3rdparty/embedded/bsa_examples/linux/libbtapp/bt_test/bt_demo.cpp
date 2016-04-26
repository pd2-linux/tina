#include <bluetooth_socket.h>
#include <unistd.h>

static int status = 0;
static int playing = 0;
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
    	  	  status = 1;
    	  	  break;
    	  }
    	  
    	  case BT_AVK_DISCONNECTED_EVT:
    	  {
    	  	  printf("Media audio disconnected!\n");
    	  	  c.set_dev_connectable(1);
    	  	  c.set_dev_discoverable(1);
    	  	  status = 0;
    	  	  break;
    	  }
    	  
    	  case BT_AVK_START_EVT:
    	  {
    	  	  printf("Media start playing!\n");
                  playing = 1;
    	  	  break;
    	  }
    	  
    	  case BT_AVK_STOP_EVT:
    	  {
    	      printf("Media stop playing!\n");
              playing = 0;
    	      break;	
    	  }
    	  
    	  default:
    	      break;
    }	
}


int main(int argc, char *args[]){
    int times = 0;
    
    c.set_callback(bt_event_f);
     
    printf("bt off before on\n");
    c.bt_off();

    if(argc >= 2){
       c.bt_on(args[1]);
    } else {
       c.bt_on(NULL); 
    }
    
    while(status == 0){
    	printf("wait bt connected!\n");
        usleep(1000*1000);
    }
    
    
    while(1){
        usleep(2000*1000);
        if(playing == 1){
            printf("BT media playing\n");
            usleep(20000*1000);
            c.disconnect();
            //c.reset_avk_status();
        }

        if(status == 0){
             printf("BT disconnect, auto connect!\n");
             c.connect_auto();
        }

        usleep(10000*1000);

    }

/*
    while(1){
    	  usleep(20000*1000);
    	  if (status == 1){
    	      printf("BT connected, disconnect!\n");
    	      c.disconnect();
    	  }
    	  
    	  usleep(10000*1000);
    	  
        if(status == 0){
            printf("BT disconnect, auto connect!\n");
            c.connect_auto();
        }
    }
    
    c.bt_off();
    
    c.bt_on(NULL);
    printf("bt do second test!\n");
    c.do_test();
    c.set_bt_name("aw bt test001");
 
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
