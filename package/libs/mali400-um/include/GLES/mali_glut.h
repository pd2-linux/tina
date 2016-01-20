/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2005, 2007-2008, 2010, 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef MICROGLUT_H
#define MICROGLUT_H

#include <GLES/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GLUTCALLBACK

/* flags for glutInitDisplayMode() */
#define GLUT_RGB            0
#define GLUT_RGBA           GLUT_RGB
#define GLUT_SINGLE         0
#define GLUT_DOUBLE         2
#define GLUT_ALPHA          8
#define GLUT_DEPTH          16
#define GLUT_STENCIL        32
#define GLUT_MULTISAMPLE    128

void glutInit(int *argc, char **argv);
void glutInitWindowSize(int width, int height);
void glutInitWindowPosition(int x, int y);
void glutInitDisplayMode(unsigned int mode);
int  glutCreateWindow(char *name);
void glutDestroyWindow(int win);

void glutDisplayFunc(void (GLUTCALLBACK *func)(void));
void glutIdleFunc(void (GLUTCALLBACK *func)(void));
void glutReshapeFunc(void (GLUTCALLBACK *func)(int, int));
void glutKeyboardFunc(void (GLUTCALLBACK *func)(unsigned char, int, int));
void glutKeyboardUpFunc(void (GLUTCALLBACK *func)(unsigned char, int, int));

void glutMainLoop(void);
void glutSwapBuffers(void);
void glutFullScreen(void);

#ifdef __cplusplus
}
#endif

#endif /* MICROGLUT_H */
