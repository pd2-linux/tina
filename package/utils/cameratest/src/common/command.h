/*
 * \file        command.h
 * \brief       
 *
 * \version     1.0.0
 * \date        2014-6-4
 * \author      Henrisk <heweihong@allwinnertech.com>
 * 
 * Copyright (c) 2014 Allwinner Technology. All Rights Reserved.
 *
 */

#ifndef __COMMAND_H__
#define __COMMAND_H__

//command
typedef enum _command
{
	COMMAND_UNUSED = 0,
		
	//for hawkview
	COMMAND_EXIT 		= 1,
	COMMAND_WAIT 		= 2,

	//for video capture
	SET_SUB_ROT			= 145,		//eg: command string "145:90#"
	SET_CAP_INFO		= 146,		//eg: command string "146:0:1:1280x720#" video:0,s_input:1,size:1280x720
	SET_CAP_VIDEO		= 147,		//eg: command string "147:0:1#"   video:0,s_input:1
	SET_CAP_SIZE		= 148,		//eg: command string "148:1280x720#"
	SAVE_IMAGE			= 149,		//eg: command string "149:image_1.yuv#"
	SAVE_FRAME 			= 150,		//eg: command string "150:5#" 5:save framerate 5fps	
	STOP_SAVE_FRAME		= 151,		//eg: command string "151#"
	
	STOP_STREAMMING 	= 160,
	START_STREAMMING	= 161,


	//for display	
	FULL_SCREEN 		= 200,
	FULL_CAPTURE 		= 201,
	
}command;
#endif
