
#ifndef ENGINE_IOS_COMMON_H
#define ENGINE_IOS_COMMON_H

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include <stdio.h>

#define DBG_LOG(x, ...) do { printf(x, __VA_ARGS__); printf("\n"); } while (0)

#endif
