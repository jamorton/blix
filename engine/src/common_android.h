
#ifndef ENGINE_ANDROID_COMMON_H
#define ENGINE_ANDROID_COMMON_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>

#define DBG_LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "game-engine", __VA_ARGS__))

#endif
