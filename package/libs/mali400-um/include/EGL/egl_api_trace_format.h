/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2010-2011 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _EGL_API_TRACE_FORMAT_H_
#define _EGL_API_TRACE_FORMAT_H_

/**
 * Indices within an array of native pixmap attribute values.
 */
enum
{
	EGL_PIXMAP_ATTRIB_HANDLE,
	EGL_PIXMAP_ATTRIB_UMP,
	EGL_PIXMAP_ATTRIB_WIDTH,
	EGL_PIXMAP_ATTRIB_HEIGHT,
	EGL_PIXMAP_ATTRIB_PITCH,
	EGL_PIXMAP_ATTRIB_PIXEL_FORMAT,
	EGL_PIXMAP_ATTRIB_TEXEL_FORMAT,
	EGL_PIXMAP_ATTRIB_PIXEL_LAYOUT,
	EGL_PIXMAP_ATTRIB_TEXEL_LAYOUT,
	EGL_PIXMAP_ATTRIB_RED_BLUE_SWAP,
	EGL_PIXMAP_ATTRIB_REVERSE_ORDER,
	EGL_PIXMAP_ATTRIB_LAST /* must be last to give the no. of attributes */
};

/**
 * Indices within an array of native windows attribute values.
 */
enum
{
	EGL_WINDOW_ATTRIB_HANDLE,
	EGL_WINDOW_ATTRIB_WIDTH,
	EGL_WINDOW_ATTRIB_HEIGHT,
	EGL_WINDOW_ATTRIB_LAST /* must be last to give the no. of attributes */
};

#endif /* _EGL_API_TRACE_FORMAT_H_ */
