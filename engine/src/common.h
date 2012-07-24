
#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <functional>
#include <cassert>

// Android Device
#if defined(__ANDROID__)

  #define IS_ANDROID 1
  #include "common_android.h"

// Apple Device
#elif defined(__APPLE__)

  #define IS_IOS 1
  #include "common_ios.h"

#else

  #error INVALID PLATFORM

#endif

typedef unsigned int uint;

#define hash_table std::unordered_map

inline void die(const char * msg)
{
    DBG_LOG("ERROR: %s\n", msg);
    abort();
}

namespace Math {

    template <typename T>
    inline const T& min(const T& a, const T& b) { return a < b ? a : b; }

    template <typename T>
    inline const T& max(const T& a, const T& b) { return a > b ? a : b; }

    inline float random() { return rand() / (float)RAND_MAX; }
    inline int randRange(int min, int max) {
        return rand() % (max - min) + min;
    }

    template <typename T>
    inline const T& clamp(const T& v, const T& minv, const T& maxv)
    {
        return max(minv, min(v, maxv));
    }

}

static inline uint8_t floatToU8(float v)
{
    return (uint8_t)floor(v*255);
}

static inline float u8ToFloat(uint8_t v)
{
    return (float)v / 255.0;
}

typedef uint Color;

#define memberFunc(f) std::bind(&f, this)
#define methodListener(f) std::bind(&f, this, std::placeholders::_1)

#ifdef DEBUG
#define DBG_ASSERT(cond) assert(cond)
#else
#define DBG_ASSERT(COND)
#endif

#endif // ENGINE_COMMON_H
