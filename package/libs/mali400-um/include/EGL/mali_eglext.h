/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2008-2011, 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _EGLEXT_H_
#define _EGLEXT_H_

#include <EGL/eglext.h>

/* At least up to Android 4.2.2_mr1 the eglext.h in Android doesn't
 * have the EGL_KHR_wait_sync types, only corresponding
 * EGL_ANDROID_wait_sync types */
#ifndef EGL_KHR_wait_sync
#define EGL_KHR_wait_sync 1
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLint EGLAPIENTRY eglWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLint(EGLAPIENTRYP PFNEGLWAITSYNCKHRPROC)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags);
#endif

/* eglWaitSyncANDROID is not in eglext.h of Android 4.3 (jellybean_mr2)
 * EGL_ANDROID_wait_sync */
#ifndef EGL_ANDROID_wait_sync
#define EGL_ANDROID_wait_sync
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLint EGLAPIENTRY eglWaitSyncANDROID(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags);
#endif
typedef EGLint(EGLAPIENTRYP PFNEGLWAITSYNCANDROID)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags);
#endif

/* EGL_ANDROID_blob_cache extension */
#ifndef EGL_ANDROID_blob_cache
#define EGL_ANDROID_blob_cache 1
typedef khronos_ssize_t EGLsizeiANDROID;
typedef void (*EGLSetBlobFuncANDROID)(const void *key, EGLsizeiANDROID keySize, const void *value, EGLsizeiANDROID valueSize);
typedef EGLsizeiANDROID(*EGLGetBlobFuncANDROID)(const void *key, EGLsizeiANDROID keySize, void *value, EGLsizeiANDROID valueSize);
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI void EGLAPIENTRY eglSetBlobCacheFuncsANDROID(EGLDisplay dpy, EGLSetBlobFuncANDROID set, EGLGetBlobFuncANDROID get);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef void (EGLAPIENTRYP PFNEGLSETBLOBCACHEFUNCSANDROIDPROC)(EGLDisplay dpy, EGLSetBlobFuncANDROID set, EGLGetBlobFuncANDROID get);
#endif

/* Part of EGL_create_context extension. We currently support only debug
 * context creation flag */
#ifndef EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR               0x00000001
#endif

/* Tokens introduced by EGL_KHR_create_context extension */
#ifndef EGL_KHR_create_context
#define EGL_KHR_create_context 1
#define EGL_CONTEXT_MAJOR_VERSION_KHR                       EGL_CONTEXT_CLIENT_VERSION
#define EGL_CONTEXT_MINOR_VERSION_KHR                       0x30FB
#define EGL_CONTEXT_FLAGS_KHR                               0x30FC
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR                 0x30FD
#define EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR  0x31BD
#define EGL_NO_RESET_NOTIFICATION_KHR                       0x31BE
#define EGL_LOSE_CONTEXT_ON_RESET_KHR                       0x31BF
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR                    0x00000001
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR       0x00000002
#define EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR            0x00000004
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR             0x00000001
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR    0x00000002
#define EGL_OPENGL_ES3_BIT_KHR                              0x00000040
#endif

#endif /* _EGLEXT_H_ */
