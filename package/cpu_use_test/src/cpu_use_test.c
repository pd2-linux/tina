#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

int Show(char *framebuffer_devices)
{
	int fp = 0;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	int screensize = 0;
	char *fbp = 0;
	int x = 0, y = 0;
	int location = 0;
	int bytes_per_pixel;			// 组成每一个像素点的字节数
	
	int count = 0;
	

	fp = open(framebuffer_devices, O_RDWR);
	if(fp < 0)
	{
		printf("error: Can not open %s device!!!!!!!!!!!!!!!!!!!!!!!!!!\n", framebuffer_devices);
		return -1;
	}
	
	if(ioctl(fp, FBIOGET_FSCREENINFO, &finfo))
	{
		printf("err FBIOGET_FSCREENINFO!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		close(fp);
		return -2;
	}
	
	if(ioctl(fp, FBIOGET_VSCREENINFO, &vinfo))
	{
		printf("err FBIOGET_VSCREENINFO!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		close(fp);
		return -3;
	}
	
	bytes_per_pixel = vinfo.bits_per_pixel / 8;
	
	screensize = vinfo.xres * vinfo.yres * bytes_per_pixel;
	
	printf("x = %d  y = %d bytes_per_pixel = %d\n", vinfo.xres, vinfo.yres, bytes_per_pixel);
	printf("screensize = %d\n", screensize);
	
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
	if((int) fbp == -1)
	{
		printf("err mmap!!!!!!!!!!!!!!!!!!!!!!!!!!");
		close(fp);
		return -4;
	}

	for(count = 0; count <100; count ++)
	{
		for(x = 0; x < vinfo.xres; x ++)
		{
			for(y = 0; y < vinfo.yres; y++)
			{
				location = x * bytes_per_pixel + y * finfo.line_length;
				*(fbp + location) = 255;				// 红色的色深
				*(fbp + location + 1) = 00;			// 绿色的色深
				*(fbp + location + 2) = 00;		// 蓝色的色深
			//	*(fbp + location + 3) = 0;			// 是否透明
			}
		}
		//usleep(500 * 1000);
		sleep(1);
		for(x = 0; x < vinfo.xres; x ++)
		{
			for(y = 0; y < vinfo.yres; y++)
			{
				location = x * bytes_per_pixel + y * finfo.line_length;
				*(fbp + location) = 00;				// 红色的色深
				*(fbp + location + 1) = 255;			// 绿色的色深
				*(fbp + location + 2) = 00;		// 蓝色的色深
			//	*(fbp + location + 3) = 0;			// 是否透明
			}
		}
		//usleep(500 * 1000);
		sleep(1);
		for(x = 0; x < vinfo.xres; x ++)
		{
			for(y = 0; y < vinfo.yres; y++)
			{
				location = x * bytes_per_pixel + y * finfo.line_length;
				*(fbp + location) = 00;				// 红色的色深
				*(fbp + location + 1) = 00;			// 绿色的色深
				*(fbp + location + 2) = 255;		// 蓝色的色深
			//	*(fbp + location + 3) = 0;			// 是否透明
			}
		}
		sleep(1);
		
		for(x = 0; x < vinfo.xres; x ++)
		{
			for(y = 0; y < vinfo.yres; y++)
			{
				location = x * bytes_per_pixel + y * finfo.line_length;
				*(fbp + location) = 58;				// 红色的色深
				*(fbp + location + 1) = 107;			// 绿色的色深
				*(fbp + location + 2) = 165;		// 蓝色的色深
			//	*(fbp + location + 3) = 0;			// 是否透明
			}
		}
		sleep(1);
	}
	
	munmap(fbp, screensize);
	close(fp);
	
	return 0;
}

int main(void)
{
	char fb_path[30] = {0};
	int i = 0;

	Show("/dev/fb0");
	#if 0
	for(i = 0; i < 100; i ++)
	{
		memset(fb_path, 0, sizeof(fb_path));
		sprintf(fb_path, "/dev/fb%d", i);
		printf("show %s...\n", fb_path);
		if(-1 == Show(fb_path))
			break;
		printf("%s ok\n\n", fb_path);
	}
	#endif
	return 0;
}
