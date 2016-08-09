/*
 * uart_test.c
 * (C) Copyright 2014-2019
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * huangwei <huangwei@allwinnertech.com>
 *
 * sunxi uart test
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/wait.h>

#define CMD_READ			0
#define CMD_WRITE			1
#define CMD_SET_PARAM			2
#define CMD_SET_PARAM_WHEN_TX		3
#define CMD_SET_PARAM_WHEN_RX		4

#define TEST_DATA_LEN			256

#define DBG(string, args...)		\
	do { 				\
		printf("UART> %s(%u): ", __FUNCTION__, __LINE__); \
		printf(string, ##args); \
	} while(0)
	
#define DBG_RESULT(ret)			\
	do { 				\
		if (0 == ret)		\
			printf("UART> %s success\n", __FUNCTION__); \
		else			\
			printf("UART> %s failed\n", __FUNCTION__); \
	} while(0)
	
typedef struct {
	int baud;
	int databits;
	int parity;
	int stopbits;
	int flow_ctrl;	
}st_uart_param;

/**
 * print_usage
 */
void print_usage()
{
	printf("*************************************\n");
	printf("  A Simple Serial Port Test Utility\n");
	printf("*************************************\n\n");

	printf("Usage:\n");
	printf("  ./uarttest <cmd> <name1> <baud> <databits> ");
	printf("<parity> <stopbits> <flow_ctrl> <cmd_name>\n");
	printf("\tcmd: 0(read), 1(write), 2(set param), "); 
	printf("3(set param when tx), 4(set param when rx)\n");
       	printf("\tdatabits: 5, 6, 7, 8\n");
       	printf("\tparity: 0(none), 1(odd), 2(even)\n");
	printf("\tstopbits: 1, 2\n");
	printf("\tflow_ctrl: 0(no), 1(hardware), 2(software)\n");
	printf("\tname2: optional\n");
	
	printf("Example:\n");
  	printf("  ./uarttest 2 /dev/ttyS1 9600 8 0 1 0\n");
}

/**
 * print_data
 * @buf:
 * @len:
 */
void print_data(char *buf, int len)
{
	int i;
	
	for (i=0; i<len; i++)
		printf("0x%02x ", buf[i]);
	printf("\n");
}

/**
 * uart_open
 * @name:
 *
 *
 */
int uart_open(char *name)
{
	int fd = -1;
	
	DBG("open uart %s\n", name);
		  
  	if (strcmp(name, "/dev/ttyS0") == 0) {
		DBG("unsupported uart /dev/ttyS0\n");
		return -1;
	}
	
	fd = open(name, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd) {
		DBG("unsupported uart %s\n", name);
	}
	
	if(fcntl(fd, F_SETFL, 0) < 0) {
		DBG("fcntl failed/n");
		close(fd);
		return -1;
	}  

	DBG("open uart %s success\n", name);

	return fd;

}

/**
 * uart_set
 * @name:
 * @param:
 *
 *
 */
int uart_set(int fd, st_uart_param *param)
{
	int i = 0;
	struct termios options;
	int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, 
			   B2400, B1800, B1200, B600, B300, B200, B150, 
			   B134, B110, B75, B50, B0, B9600};
	int name_arr[] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1800, 
			  1200, 600, 300, 200, 150, 134, 110, 75, 50, 0};
	
	DBG("set port (baud, databits, parity, stopbits, flow_ctrl)(%d, %d, %d, %d, %d)\n", 
		param->baud, param->databits, param->parity, param->stopbits, param->flow_ctrl);
			
	if (tcgetattr(fd, &options) != 0) {
		DBG("tcgetattr failed\n");
		goto SET_PARAM_FAIL;
	}

	for (i= 0; i < sizeof(name_arr)/sizeof(int); i++) {
		if (param->baud == name_arr[i])
			break;
	}
	
	if (i == sizeof(speed_arr)/sizeof(int))
		DBG("unsupported baudrate %d, use 9600 instead\n", param->baud);
		
	cfsetispeed(&options, speed_arr[i]);
	cfsetospeed(&options, speed_arr[i]);
	
	options.c_cflag |= CLOCAL;
	options.c_cflag |= CREAD;
	
	options.c_cflag &= ~CSIZE;
	switch (param->databits) {  
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:    
		options.c_cflag |= CS7;
		break;
	case 8:    
		options.c_cflag |= CS8;
		break;
	default:   
		DBG("unsupported data size %d\n", param->databits);
		goto SET_PARAM_FAIL;
	}
	
	switch (param->parity) {
	case 0:
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;  
	case 1:
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;  
	case 2:
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	default:   
		DBG("unsupported parity %d\n", param->parity);    
		goto SET_PARAM_FAIL; 
	}  

	switch (param->stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		DBG("unsupported stop bits %d\n", param->stopbits);
		goto SET_PARAM_FAIL;
	}
	
	switch (param->flow_ctrl) {
	case 0:
		options.c_cflag &= ~CRTSCTS;
		break;
	case 1:
		options.c_cflag |= CRTSCTS;
		break;
#if 0
	case 2:
		options.c_cflag |= IXON | IXOFF | IXANY;
		options.c_cc[VSTART] = 0x11;  
		options.c_cc[VSTOP] = 0x13; 
		break;
#endif
	default:
		DBG("unsupported flow contrl %d\n", param->flow_ctrl);
		goto SET_PARAM_FAIL;
	}
	
	options.c_iflag &= ~ (IXON | IXOFF | IXANY);
	options.c_iflag &= ~ (INLCR | ICRNL | IGNCR);
	options.c_oflag &= ~(ONLCR | OCRNL);
	
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag  &= ~OPOST;

	options.c_cc[VTIME] = 10;
	options.c_cc[VMIN] = 0;

	tcflush(fd, TCIFLUSH);
	
	if (tcsetattr(fd, TCSANOW, &options) != 0) { 
		DBG("tcsetattr failed\n");   
		goto SET_PARAM_FAIL; 
	}
	
	DBG("set param success\n");
	return 0;
	
SET_PARAM_FAIL:
	DBG("set param failed\n");
	print_usage();
	return -1;
}

/**
 * uart_read
 * @fd:
 * @buf:
 * @data_len:
 *
 *
 */
int uart_read(int fd, char *buf, int data_len)
{
#if 0
	int total_len = 0;
	int read_len = 0;
	
	DBG("want read data len %dB\n", data_len);
	
	while(data_len > 0) {
		read_len = read(fd, buf, data_len);
		if (read_len > 0) {
	     		data_len -= read_len;
	       		total_len += read_len;     
	       		buf += read_len;
	       	} else
	       		break;
	}

	DBG("actual read data len %dB: \n", total_len);
	print_data(buf, total_len);
		
	return total_len;
	
#else
	int total_len = 0;
	int read_len = 0;
	int fs_sel = 0;  
    	fd_set fs_read;  
     	struct timeval time;  
     
    	FD_ZERO(&fs_read);  
    	FD_SET(fd, &fs_read);  
     
   	time.tv_sec = 10;  
    	time.tv_usec = 0;  
     
     	DBG("want read data len %dB\n", data_len);
     	
    	while (data_len > 0) {
 	    	fs_sel = select(fd+1, &fs_read, NULL, NULL, &time);  
	    	if(fs_sel) {  
	              	read_len = read(fd, buf, data_len); 
	               	data_len -= read_len;
	              	total_len += read_len;     
	              	buf += read_len;
	        }
	        else
	        	break;
	}
	
	DBG("actual read data len %dB: \n", total_len);
	print_data(buf, total_len);	           
	
	return total_len; 
#endif
	
}

/**
 * uart_write
 * @fd:
 * @buf:
 * @data_len:
 *
 * 
 */ 
int uart_write(int fd, char *buf, int data_len)
{
	int len = 0;	
	
	DBG("want write data %dB: \n", data_len);
	print_data(buf, data_len);
	
	len = write(fd, buf, data_len);
	
	if (len != data_len) {
		tcflush(fd, TCOFLUSH);
		return -1;
	}
	
	DBG("actual write data %dB\n", len);
		
	return len;
}

/**
 * cnd_set_param
 * @name1:
 * @name2:
 * @cmd:
 * @param:
 *
 * success:return 0; fail: return -1
 */
int cmd_transfer(char *name1, char *cmd_name, int cmd, st_uart_param *param)
{
	int i;
	int fd1 = -1;
	int fd2 = -1;
	char send_buf[TEST_DATA_LEN];
	char recv_buf[TEST_DATA_LEN];
	int len;
	int cmd_name_len;
	pid_t pid = 1;		
	int cmp = 0;
	int status = -1;
	int ret = -1;

	fd1 = uart_open(name1);
	if (-1 == fd1)			
		goto TRANSFER_END;
		
	if (-1 == uart_set(fd1, param)) {
		close(fd1);
		goto TRANSFER_END;
	}	
	cmd_name_len = strlen(cmd_name) + 1;

	if (CMD_READ == cmd) {
		strlcpy(recv_buf, cmd_name, cmd_name_len);
		printf("%s,l:%d, cmd_name:%s, recv_buf:%s\n", __func__, __LINE__, cmd_name, recv_buf);
	} else {
		strlcpy(send_buf, cmd_name, cmd_name_len);
		printf("%s,l:%d, cmd_name:%s, send_buf:%s\n", __func__, __LINE__, cmd_name, send_buf);
	}

	if (1 == pid) {
		printf("origin process:%s,l:%d\n", __func__, __LINE__);
		if (CMD_READ == cmd)
			len = uart_read(fd1, recv_buf, cmd_name_len);
		else {
			len = uart_write(fd1, send_buf, cmd_name_len);
			uart_read(fd1, recv_buf, cmd_name_len);
			printf("recv_buf:%s\n", recv_buf);
		}
		if (cmd_name_len == len) 
			ret = 0;
	}
TRANSFER_END:
	DBG_RESULT(ret);
	if (-1 != fd1)
		close(fd1);
	if (-1 != fd2)
		close(fd2);	
	return ret;
}

/**
 * cmd_set_param
 * @name:
 * @param:
 *
 *  success:return 0; fail: return -1
 */
int cmd_set_param(char *name, st_uart_param *param)
{
	int fd;
	int ret = -1;
		
	fd = uart_open(name);
	if (-1 == fd) 
		return -1;
	
	ret = uart_set(fd, param);

	close(fd);
	
	DBG_RESULT(ret);
	return ret;
}

int main(int argc, char **argv)
{
	int cmd;
	char *name1 = NULL;
	char *cmd_name = NULL;
	st_uart_param uart_param;
	int ret = -1;
	
	if (argc < 8) {
		print_usage();
		return -1;
	} 
	
	cmd = atoi(argv[1]);
	name1 = argv[2];
	uart_param.baud = atoi(argv[3]);
	uart_param.databits = atoi(argv[4]);
	uart_param.parity = atoi(argv[5]);
	uart_param.stopbits = atoi(argv[6]);
	uart_param.flow_ctrl = atoi(argv[7]);
	if (argc > 8)
		cmd_name = argv[8];

	switch (cmd) {
	case CMD_READ:
		ret = cmd_transfer(name1, cmd_name, CMD_READ, &uart_param);
		break;

	case CMD_WRITE:
		ret = cmd_transfer(name1, cmd_name, CMD_WRITE, &uart_param);
		break;

	case CMD_SET_PARAM:
		ret = cmd_set_param(name1, &uart_param);
		break;
		
	default:
		DBG("unsupported cmd %d\n", cmd);
		break;
	}

	return ret;
}
