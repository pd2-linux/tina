#ifndef SUB_BASE_H
#define SUB_BASE_H

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

//#include "OMX_Types.h"
#include "transfromType.h"

OMX_S32 open_stream(OMX_S32 fd, OMX_S8** options, OMX_S32* file_format);
void free_stream(OMX_S32 fd);
OMX_U8* stream_read_line(OMX_S32 fd,OMX_S64 nFdOffset, OMX_S64 nFdLen, OMX_U8* mem, OMX_S32 max, OMX_S32 utf16);

#endif//SUB_BASE_H
