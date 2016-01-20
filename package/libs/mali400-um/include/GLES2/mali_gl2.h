/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2007-2010, 2012-2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _MALI_GL2_H_
#define _MALI_GL2_H_


/*
 * This is the Mali gl wrapper, for use in driver development only
 *  all applications should be built with the stock gl.h and egl.h
 */

/* current khronos distributed gl.h, must be on include path */
#if MALI_USE_GLES_1 && MALI_USE_GLES_2
#include <GLES_GLES2/mali_gl_gl2.h>
#else
/**
 * New revisions of gl.h have changed glShaderSource to accept a
 * const GLchar* const * parameter instead of a const GLChar ** parameter.
 * Since it is impossible to know which header the users will be supplying,
 * we need to remove the definition of this function and supply our own.
 * No matter which is used, they are binary compatible, but we avoid a
 * ton of warnings this way.
 *
 * This removal happens here:
 */
#define glShaderSource _mali_override_gl_shadersource

/* then include the GLES header */
#include <GLES2/gl2.h>

/* and undo the glShaderSource redefine, replace with our own */
#undef glShaderSource
GL_APICALL void         GL_APIENTRY glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);

#endif


/* driver specific contents can be defined here */

#endif /* _MALI_GL2_H_ */
