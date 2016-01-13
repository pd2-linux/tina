#include "bluetooth_socket.h"

extern "C" int bluetooth_start(void *p, char *p_file);

int main(int argv, char * argc[])
{
	  int i=0;
    printf("print args:\n");
    for(i=0; i<argv; i++)
    {
        printf("arg %d:%s\n", i, argc[i]);
    }
    
    s_bt *p_sbt = NULL;	
    p_sbt = new s_bt();
    bluetooth_start((void *)p_sbt, argc[1]);
    while(1);	
    return 0;
}