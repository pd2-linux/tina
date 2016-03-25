#include <stdio.h>
extern int tinymp3_play(char *mp3_file);

extern void tinymp3_reset(void);

extern void tinymp3_stop(void);
int main(int argc, char *argv[])
{
	int ret;

	tinymp3_play(argv[1]);
	while(1){
		printf("sleep 1\n");
		sleep(5);
	}

	return ret;
}
