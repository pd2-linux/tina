/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2008-2010, 2012-2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */


#ifndef _MALI_GL_GL2_H_
#define _MALI_GL_GL2_H_

/*
 * This is the Mali gl wrapper, for use in driver development only
 *  all applications should be built with the stock gl.h and egl.h
 */

/* current khronos distributed gl.h, must be on include path */

#include <GLES/gl.h>

#define GLvoid       GLvoid__
#define GLchar       GLchar__
#define GLenum       GLenum__
#define GLboolean    GLboolean__
#define GLbitfield   GLbitfield__
#define GLbyte       GLbyte__
#define GLshort      GLshort__
#define GLint        GLint__
#define GLsizei      GLsizei__
#define GLubyte      GLubyte__
#define GLushort     GLushort__
#define GLuint       GLuint__
#define GLfloat      GLfloat__
#define GLclampf     GLclampf__
#define GLfixed      GLfixed__
#define GLintptr     GLintptr__
#define GLsizeiptr   GLsizeiptr__
#define glTexImage2D glTexImage2D__

/* Undefine defines that are defined in both gl.h and gl2.h */
#undef GL_DEPTH_BUFFER_BIT
#undef GL_STENCIL_BUFFER_BIT
#undef GL_COLOR_BUFFER_BIT
#undef GL_FALSE
#undef GL_TRUE
#undef GL_POINTS
#undef GL_LINES
#undef GL_LINE_LOOP
#undef GL_LINE_STRIP
#undef GL_TRIANGLES
#undef GL_TRIANGLE_STRIP
#undef GL_TRIANGLE_FAN
#undef GL_NEVER
#undef GL_LESS
#undef GL_EQUAL
#undef GL_LEQUAL
#undef GL_GREATER
#undef GL_NOTEQUAL
#undef GL_GEQUAL
#undef GL_ALWAYS
#undef GL_ZERO
#undef GL_ONE
#undef GL_SRC_COLOR
#undef GL_ONE_MINUS_SRC_COLOR
#undef GL_SRC_ALPHA
#undef GL_ONE_MINUS_SRC_ALPHA
#undef GL_DST_ALPHA
#undef GL_ONE_MINUS_DST_ALPHA
#undef GL_DST_COLOR
#undef GL_ONE_MINUS_DST_COLOR
#undef GL_SRC_ALPHA_SATURATE
#undef GL_FRONT
#undef GL_BACK
#undef GL_FRONT_AND_BACK
#undef GL_TEXTURE_2D
#undef GL_CULL_FACE
#undef GL_BLEND
#undef GL_DITHER
#undef GL_STENCIL_TEST
#undef GL_DEPTH_TEST
#undef GL_SCISSOR_TEST
#undef GL_POLYGON_OFFSET_FILL
#undef GL_SAMPLE_ALPHA_TO_COVERAGE
#undef GL_SAMPLE_COVERAGE
#undef GL_NO_ERROR
#undef GL_INVALID_ENUM
#undef GL_INVALID_VALUE
#undef GL_INVALID_OPERATION
#undef GL_OUT_OF_MEMORY
#undef GL_CW
#undef GL_CCW
#undef GL_LINE_WIDTH
#undef GL_ALIASED_POINT_SIZE_RANGE
#undef GL_ALIASED_LINE_WIDTH_RANGE
#undef GL_CULL_FACE_MODE
#undef GL_FRONT_FACE
#undef GL_DEPTH_RANGE
#undef GL_DEPTH_WRITEMASK
#undef GL_DEPTH_CLEAR_VALUE
#undef GL_DEPTH_FUNC
#undef GL_STENCIL_CLEAR_VALUE
#undef GL_STENCIL_FUNC
#undef GL_STENCIL_VALUE_MASK
#undef GL_STENCIL_FAIL
#undef GL_STENCIL_PASS_DEPTH_FAIL
#undef GL_STENCIL_PASS_DEPTH_PASS
#undef GL_STENCIL_REF
#undef GL_STENCIL_WRITEMASK
#undef GL_VIEWPORT
#undef GL_SCISSOR_BOX
#undef GL_COLOR_CLEAR_VALUE
#undef GL_COLOR_WRITEMASK
#undef GL_UNPACK_ALIGNMENT
#undef GL_PACK_ALIGNMENT
#undef GL_MAX_TEXTURE_SIZE
#undef GL_MAX_VIEWPORT_DIMS
#undef GL_SUBPIXEL_BITS
#undef GL_RED_BITS
#undef GL_GREEN_BITS
#undef GL_BLUE_BITS
#undef GL_ALPHA_BITS
#undef GL_DEPTH_BITS
#undef GL_STENCIL_BITS
#undef GL_POLYGON_OFFSET_UNITS
#undef GL_POLYGON_OFFSET_FACTOR
#undef GL_TEXTURE_BINDING_2D
#undef GL_SAMPLE_BUFFERS
#undef GL_SAMPLES
#undef GL_SAMPLE_COVERAGE_VALUE
#undef GL_SAMPLE_COVERAGE_INVERT
#undef GL_NUM_COMPRESSED_TEXTURE_FORMATS
#undef GL_COMPRESSED_TEXTURE_FORMATS
#undef GL_DONT_CARE
#undef GL_FASTEST
#undef GL_NICEST
#undef GL_GENERATE_MIPMAP_HINT
#undef GL_BYTE
#undef GL_UNSIGNED_BYTE
#undef GL_SHORT
#undef GL_UNSIGNED_SHORT
#undef GL_FLOAT
#undef GL_FIXED
#undef GL_UNSIGNED_SHORT_4_4_4_4
#undef GL_UNSIGNED_SHORT_5_5_5_1
#undef GL_UNSIGNED_SHORT_5_6_5
#undef GL_KEEP
#undef GL_REPLACE
#undef GL_INCR
#undef GL_DECR
#undef GL_VENDOR
#undef GL_RENDERER
#undef GL_VERSION
#undef GL_EXTENSIONS
#undef GL_NEAREST
#undef GL_LINEAR
#undef GL_NEAREST_MIPMAP_NEAREST
#undef GL_LINEAR_MIPMAP_NEAREST
#undef GL_NEAREST_MIPMAP_LINEAR
#undef GL_LINEAR_MIPMAP_LINEAR
#undef GL_TEXTURE_MAG_FILTER
#undef GL_TEXTURE_MIN_FILTER
#undef GL_TEXTURE_WRAP_S
#undef GL_TEXTURE_WRAP_T
#undef GL_TEXTURE0
#undef GL_TEXTURE1
#undef GL_TEXTURE2
#undef GL_TEXTURE3
#undef GL_TEXTURE4
#undef GL_TEXTURE5
#undef GL_TEXTURE6
#undef GL_TEXTURE7
#undef GL_TEXTURE8
#undef GL_TEXTURE9
#undef GL_TEXTURE10
#undef GL_TEXTURE11
#undef GL_TEXTURE12
#undef GL_TEXTURE13
#undef GL_TEXTURE14
#undef GL_TEXTURE15
#undef GL_TEXTURE16
#undef GL_TEXTURE17
#undef GL_TEXTURE18
#undef GL_TEXTURE19
#undef GL_TEXTURE20
#undef GL_TEXTURE21
#undef GL_TEXTURE22
#undef GL_TEXTURE23
#undef GL_TEXTURE24
#undef GL_TEXTURE25
#undef GL_TEXTURE26
#undef GL_TEXTURE27
#undef GL_TEXTURE28
#undef GL_TEXTURE29
#undef GL_TEXTURE30
#undef GL_TEXTURE31
#undef GL_ACTIVE_TEXTURE
#undef GL_REPEAT
#undef GL_CLAMP_TO_EDGE
#undef GL_ARRAY_BUFFER
#undef GL_ELEMENT_ARRAY_BUFFER
#undef GL_ARRAY_BUFFER_BINDING
#undef GL_ELEMENT_ARRAY_BUFFER_BINDING
#undef GL_STATIC_DRAW
#undef GL_DYNAMIC_DRAW
#undef GL_BUFFER_SIZE
#undef GL_BUFFER_USAGE

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


#include <GLES2/gl2.h>

/* and undo the glShaderSource redefine, replace with our own */
#undef glShaderSource
GL_APICALL void         GL_APIENTRY glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);

#undef GLvoid
#undef GLchar
#undef GLenum
#undef GLboolean
#undef GLbitfield
#undef GLbyte
#undef GLshort
#undef GLint
#undef GLsizei
#undef GLubyte
#undef GLushort
#undef GLuint
#undef GLfloat
#undef GLclampf
#undef GLfixed
#undef GLintptr
#undef GLsizeiptr
#undef glTexImage2D

/* driver specific contents can be defined here */

#endif /* _MALI_GL_GL2_H_ */

