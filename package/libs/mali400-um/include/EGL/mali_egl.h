/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2008-2010 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/*
 * This is the Mali egl wrapper, for use in driver development only
 * all applications should be built with the plain gl.h and egl.h
 * available from Khronos
 */

#ifndef EGLAPIENTRY
#define EGLAPIENTRY
#endif

/*
 * if tracing is enabled, include oglestraceredef.h to change gl* to
 * DRVgl*, so that all entry points can be logged by trace library
 */
#ifdef GL_TRACE_WRAPPER
#include <GLES/oglestraceredef.h>
#endif

/* current khronos distributed egl.h, must be on include path */
#include <EGL/egl.h>
