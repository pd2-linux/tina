#include <bluetooth_socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DEBUG_MUSIC_INFO 0

static int last_status = 0;
static int status = 0;
static int playing = 0;
c_bt c;

void bt_event_f(BT_EVENT event, void *reply, int *len)
{
    switch(event)
    {
	  case BT_AVK_CONNECTED_EVT:
	  {
		  printf("Media audio connected!\n");
		  status = 1;
		  break;
	  }

	  case BT_AVK_DISCONNECTED_EVT:
	  {
		  printf("Media audio disconnected!\n");
                  printf("link down reason %d\n", *(int *)reply);
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
    int save_fd = -1, fd = -1;
    int i = 0;
    tBT_AVK_MUSIC_INFO music_info;

    c.set_callback(bt_event_f);

    printf("bt off before on\n");
    c.bt_off();

    last_status = 0;
    status = 0;
    if(argc >= 2){
       c.bt_on(args[1]);
    } else {
       c.bt_on(NULL);
    }

    c.set_bt_name("aw bt test001");

    while(1){
	usleep(2000*1000);

	/* connected */
	if ((last_status == 0) && (status == 1)){
	    c.set_dev_discoverable(0);
	    c.set_dev_connectable(0);
	    last_status = 1;
	}

	/* disconnected */
	if ((last_status == 1) && (status == 0)){
	    c.set_dev_discoverable(1);
	    c.set_dev_connectable(1);
	    last_status = 0;
	}

#if(DEBUG_MUSIC_INFO == 1)
	if(playing == 1){
	    c.avk_get_music_info(&music_info);
	    printf("Title: %s\n", music_info.title);
	    printf("Artist: %s\n", music_info.artist);
	    printf("Album: %s\n", music_info.album);
	    //printf("Time: %s\n", music_info.playing_time);
	}
#endif

    }
}
