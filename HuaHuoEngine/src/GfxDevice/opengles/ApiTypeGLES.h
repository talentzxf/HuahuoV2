#pragma once

#include <cstddef>
#include "Configuration/IntegerDefinitions.h"

#if PLATFORM_ANDROID || PLATFORM_WEBGL || PLATFORM_LUMIN
#   include <KHR/khrplatform.h>
#elif PLATFORM_IOS || PLATFORM_TVOS || PLATFORM_OSX
typedef intptr_t            khronos_intptr_t;
typedef intptr_t            khronos_ssize_t;
#else
typedef ptrdiff_t           khronos_intptr_t;
typedef ptrdiff_t           khronos_ssize_t;
#endif

typedef void                GLvoid;
typedef unsigned int        GLenum;
typedef unsigned char       GLboolean;
typedef unsigned int        GLbitfield;
typedef signed char         GLbyte;
typedef short               GLshort;
typedef int                 GLint;
typedef int                 GLsizei;
typedef unsigned char       GLubyte;
typedef unsigned short      GLushort;
typedef unsigned int        GLuint;
typedef float               GLfloat;
typedef float               GLclampf;
typedef SInt32              GLfixed;
typedef char                GLchar;
typedef khronos_intptr_t    GLintptr;
typedef khronos_ssize_t     GLsizeiptr;
typedef unsigned short      GLhalf;
typedef SInt64              GLint64;
typedef UInt64              GLuint64;
typedef struct __GLsync*    GLsync;
typedef double              GLdouble;
typedef void*               GLeglImageOES;
