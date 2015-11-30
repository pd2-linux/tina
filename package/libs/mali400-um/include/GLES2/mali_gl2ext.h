/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2006-2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _MALI_GL2EXT_H_
#define _MALI_GL2EXT_H_


/*
 * This is the Mali gl wrapper, for use in driver development only.  All
 * applications should be built with the stock gl2.h and gl2ext.h
 */

/* current khronos distributed gl.h, must be on include path */
#include <GLES2/gl2ext.h>

/* driver specific contents can be defined here */

/** ETC */
#define GL_OES_compressed_ETC1_RGB8_texture 1
#define GL_ETC1_RGB8_OES           0x8D64

/** video controls extensions */
#ifdef EXTENSION_VIDEO_CONTROLS_ARM_ENABLE
#define GL_TEXTURE_BRIGHTNESS_ARM           0x6001
#define GL_TEXTURE_CONTRAST_ARM             0x6002
#define GL_TEXTURE_SATURATION_ARM           0x6003
#endif

/* GL_OES_standard_derivatives */
#ifndef GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES                  0x8B8B
#endif

/* GL_OES_packed_depth_stencil */
#ifndef GL_OES_packed_depth_stencil
#define GL_DEPTH_STENCIL_OES                                    0x84F9
#define GL_UNSIGNED_INT_24_8_OES                                0x84FA
#define GL_DEPTH24_STENCIL8_OES                                 0x88F0
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT                                             0x80E1
#endif

/* GL_EXT_blend_minmax */
#ifndef GL_MIN_EXT
#define GL_MIN_EXT                                              0x8007
#endif
#ifndef GL_MAX_EXT
#define GL_MAX_EXT                                              0x8008
#endif

/* GL_ARM_mali_shader_binary */
#ifndef GL_ARM_mali_shader_binary
#define GL_MALI_SHADER_BINARY_ARM                               0x8F60
#endif

#ifndef GL_MALI_PROGRAM_BINARY_ARM
#define GL_MALI_PROGRAM_BINARY_ARM                              0x8F61
#endif


/* GL_OES_EGL_image_external */
#ifndef GL_OES_EGL_image_external
/* GLeglImageOES defined in GL_OES_EGL_image already. */
#define GL_TEXTURE_EXTERNAL_OES                                 0x8D65
#define GL_SAMPLER_EXTERNAL_OES                                 0x8D66
#define GL_TEXTURE_BINDING_EXTERNAL_OES                         0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES                     0x8D68
#endif

/* Extension KHR_DEBUG  */
#ifndef GL_KHR_debug
#define GL_KHR_debug
typedef void (GL_APIENTRY *GLDEBUGPROCKHR)(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const GLvoid *userParam);

GL_APICALL void GL_APIENTRY glDebugMessageControlKHR(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
GL_APICALL void GL_APIENTRY glDebugMessageInsertKHR(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
GL_APICALL void GL_APIENTRY glDebugMessageCallbackKHR(GLDEBUGPROCKHR callback, const void *userParam);
GL_APICALL GLuint GL_APIENTRY glGetDebugMessageLogKHR(GLuint count, GLsizei bufsize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
GL_APICALL void GL_APIENTRY glPushDebugGroupKHR(GLenum source, GLuint id, GLsizei length, const GLchar *message);
GL_APICALL void GL_APIENTRY glPopDebugGroupKHR(void);
GL_APICALL void GL_APIENTRY glObjectLabelKHR(GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
GL_APICALL void GL_APIENTRY glGetObjectLabelKHR(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
GL_APICALL void GL_APIENTRY glObjectPtrLabelKHR(const void *ptr, GLsizei length, const GLchar *label);
GL_APICALL void GL_APIENTRY glGetObjectPtrLabelKHR(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
GL_APICALL void GL_APIENTRY glGetPointervKHR(GLenum pname, void **params);

#endif

/* Tokens accepted by the <target> parameters of Enable, Disable, and IsEnabled:*/
#ifndef GL_DEBUG_OUTPUT_KHR
#define GL_DEBUG_OUTPUT_KHR                   0x92E0
#endif

#ifndef GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR       0x8242
#endif

/*Tokens accepted by the <value> parameters of GetBooleanv, GetIntegerv,
     GetFloatv, GetDoublev and GetInteger64v:*/
#ifndef GL_MAX_DEBUG_MESSAGE_LENGTH_KHR
#define GL_MAX_DEBUG_MESSAGE_LENGTH_KHR       0x9143
#endif

#ifndef GL_MAX_DEBUG_LOGGED_MESSAGES_KHR
#define GL_MAX_DEBUG_LOGGED_MESSAGES_KHR      0x9144
#endif

#ifndef GL_DEBUG_LOGGED_MESSAGES_KHR
#define GL_DEBUG_LOGGED_MESSAGES_KHR          0x9145
#endif

#ifndef GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR 0x8243
#endif

#ifndef GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR   0x826C
#endif

#ifndef GL_DEBUG_GROUP_STACK_DEPTH_KHR
#define GL_DEBUG_GROUP_STACK_DEPTH_KHR       0x826D
#endif

#ifndef GL_MAX_LABEL_LENGTH_KHR
#define GL_MAX_LABEL_LENGTH_KHR              0x82E8
#endif

/* Tokens accepted by the <pname> parameter of GetPointerv: */
#ifndef GL_DEBUG_CALLBACK_FUNCTION_KHR
#define GL_DEBUG_CALLBACK_FUNCTION_KHR       0x8244
#endif

#ifndef GL_DEBUG_CALLBACK_USER_PARAM_KHR
#define GL_DEBUG_CALLBACK_USER_PARAM_KHR     0x8245
#endif

/*Tokens accepted or provided by the <source> parameters of
      DebugMessageControl, DebugMessageInsert and DEBUGPROC, and the <sources>
      parameter of GetDebugMessageLog:*/
#ifndef GL_DEBUG_SOURCE_API_KHR
#define GL_DEBUG_SOURCE_API_KHR              0x8246
#endif

#ifndef GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR    0x8247
#endif

#ifndef GL_DEBUG_SOURCE_SHADER_COMPILER_KHR
#define GL_DEBUG_SOURCE_SHADER_COMPILER_KHR  0x8248
#endif

#ifndef GL_DEBUG_SOURCE_THIRD_PARTY_KHR
#define GL_DEBUG_SOURCE_THIRD_PARTY_KHR      0x8249
#endif

#ifndef GL_DEBUG_SOURCE_APPLICATION_KHR
#define GL_DEBUG_SOURCE_APPLICATION_KHR      0x824A
#endif

#ifndef GL_DEBUG_SOURCE_OTHER_KHR
#define GL_DEBUG_SOURCE_OTHER_KHR            0x824B
#endif

/* Tokens accepted or provided by the <type> parameters of
    DebugMessageControl, DebugMessageInsert and DEBUGPROC, and the <types>
    parameter of GetDebugMessageLog: */
#ifndef GL_DEBUG_TYPE_ERROR_KHR
#define GL_DEBUG_TYPE_ERROR_KHR             0x824C
#endif

#ifndef GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR 0x824D
#endif

#ifndef GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR  0x824E
#endif

#ifndef GL_DEBUG_TYPE_PORTABILITY_KHR
#define GL_DEBUG_TYPE_PORTABILITY_KHR       0x824F
#endif

#ifndef GL_DEBUG_TYPE_PERFORMANCE_KHR
#define GL_DEBUG_TYPE_PERFORMANCE_KHR       0x8250
#endif

#ifndef GL_DEBUG_TYPE_OTHER_KHR
#define GL_DEBUG_TYPE_OTHER_KHR             0x8251
#endif

#ifndef GL_DEBUG_TYPE_MARKER_KHR
#define GL_DEBUG_TYPE_MARKER_KHR            0x8268
#endif

/* Tokens accepted or provided by the <type> parameters of
    DebugMessageControl and DEBUGPROC, and the <types> parameter of
    GetDebugMessageLog: */
#ifndef GL_DEBUG_TYPE_PUSH_GROUP_KHR
#define GL_DEBUG_TYPE_PUSH_GROUP_KHR       0x8269
#endif

#ifndef GL_DEBUG_TYPE_POP_GROUP_KHR
#define GL_DEBUG_TYPE_POP_GROUP_KHR        0x826A
#endif

/* Tokens accepted or provided by the <severity> parameters of
    DebugMessageControl, DebugMessageInsert and DEBUGPROC callback functions,
    and the <severities> parameter of GetDebugMessageLog: */
#ifndef GL_DEBUG_SEVERITY_HIGH_KHR
#define GL_DEBUG_SEVERITY_HIGH_KHR         0x9146
#endif

#ifndef GL_DEBUG_SEVERITY_MEDIUM_KHR
#define GL_DEBUG_SEVERITY_MEDIUM_KHR       0x9147
#endif

#ifndef GL_DEBUG_SEVERITY_LOW_KHR
#define GL_DEBUG_SEVERITY_LOW_KHR          0x9148
#endif

#ifndef GL_DEBUG_SEVERITY_NOTIFICATION_KHR
#define GL_DEBUG_SEVERITY_NOTIFICATION_KHR 0x826B
#endif

/* Returned by GetError: */
#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW                 0x0504
#endif

#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW                  0x0503
#endif

/* Tokens accepted or provided by the <identifier> parameters of
    ObjectLabel and GetObjectLabel: */
#ifndef GL_BUFFER_KHR
#define GL_BUFFER_KHR                      0x82E0
#endif

#ifndef GL_SHADER_KHR
#define GL_SHADER_KHR                      0x82E1
#endif

#ifndef GL_PROGRAM_KHR
#define GL_PROGRAM_KHR                     0x82E2
#endif

/* end extension KHR_DEBUG*/

#ifndef GL_EXT_MULTISAMPLED_RENDER_TO_TEXTURE
#define GL_EXT_MULTISAMPLED_RENDER_TO_TEXTURE

/**
 * Note: The specification for GL_EXT_MULTISAMPLED_RENDER_TO_TEXTURE was released with incorrect
 * ENUM values for its four #defines. The range 0x91xx is reserved for vendor-specific defines,
 * and this is an EXT extension. The spec was changed by Khronos, but not before a mali driver
 * was shipped supporting the 0x91xx values.
 *
 * To make matters worse, the changed #defines had not only different values, but also different names.
 *
 * We basically need to support both the new and old values and names. The names are defined by the
 * programmer's GL header version, and doesn't really matter - but the values are important.
 * To ensure this, they are both wrapped up in MALI_* enums here, and all driver code must handle both!
 *
 * The old set
 * #define GL_RENDERBUFFER_SAMPLES_EXT                             0x9133
 * #define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT               0x9134
 * #define GL_MAX_SAMPLES_EXT                                      0x9135
 * #define GL_TEXTURE_SAMPLES_EXT                                  0x9136
 *
 * The new set
 * #define GL_RENDERBUFFER_SAMPLES_EXT                             0x8CAB
 * #define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT               0x8D56
 * #define GL_MAX_SAMPLES_EXT                                      0x8D57
 * #define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT           0x8D6C
 */

#define MALI_GL_RENDERBUFFER_SAMPLES_EXT_V1                        0x9133
#define MALI_GL_RENDERBUFFER_SAMPLES_EXT_V2                        0x8CAB
#define MALI_GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT_V1          0x9134
#define MALI_GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT_V2          0x8D56
#define MALI_GL_MAX_SAMPLES_EXT_V1                                 0x9135
#define MALI_GL_MAX_SAMPLES_EXT_V2                                 0x8D57
#define MALI_GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT_V1      0x9136
#define MALI_GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT_V2      0x8D6C

GL_APICALL void GL_APIENTRY glRenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glFramebufferTexture2DMultisampleEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);

#endif

/* GL_EXT_discard_framebuffer */
#ifndef GL_EXT_DISCARD_FRAMEBUFFER
#define GL_EXT_DISCARD_FRAMEBUFFER

#define GL_COLOR_EXT                               0x1800
#define GL_DEPTH_EXT                               0x1801
#define GL_STENCIL_EXT                             0x1802

GL_APICALL void GL_APIENTRY glDiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum *attachments);

#endif

/* GL_EXT_robustness */
#ifndef GL_EXT_robustness
#define GL_EXT_robustness 1
/* reuse GL_NO_ERROR */
#define GL_GUILTY_CONTEXT_RESET_EXT                             0x8253
#define GL_INNOCENT_CONTEXT_RESET_EXT                           0x8254
#define GL_UNKNOWN_CONTEXT_RESET_EXT                            0x8255
#define GL_CONTEXT_ROBUST_ACCESS_EXT                            0x90F3
#define GL_RESET_NOTIFICATION_STRATEGY_EXT                      0x8256
#define GL_LOSE_CONTEXT_ON_RESET_EXT                            0x8252
#define GL_NO_RESET_NOTIFICATION_EXT                            0x8261

#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL GLenum GL_APIENTRY glGetGraphicsResetStatusEXT(void);
GL_APICALL void GL_APIENTRY glReadnPixelsEXT(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
GL_APICALL void GL_APIENTRY glGetnUniformfvEXT(GLuint program, GLint location, GLsizei bufSize, float *params);
GL_APICALL void GL_APIENTRY glGetnUniformivEXT(GLuint program, GLint location, GLsizei bufSize, GLint *params);
#endif
typedef GLenum(GL_APIENTRYP PFNGLGETGRAPHICSRESETSTATUSEXTPROC)(void);
typedef void (GL_APIENTRYP PFNGLREADNPIXELSEXTPROC)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMFVEXTPROC)(GLuint program, GLint location, GLsizei bufSize, float *params);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMIVEXTPROC)(GLuint program, GLint location, GLsizei bufSize, GLint *params);
#endif

#endif /* _MALI_GL2EXT_H_ */
