
#ifndef ENGINE_SKIA_H
#define ENGINE_SKIA_H

#include <vector>

#include "SkTypeface.h"
#include "SkStream.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkMatrix.h"
#include "SkPoint.h"

#define alphaU8Mul(a, b) ((uint8_t)(((uint32_t)(a) * (uint32_t)(b)) / 255))

class SkDevice;

class Canvas : public SkCanvas
{


public:

    Canvas(SkDevice * dev) : SkCanvas(dev), _alphaIdx(0)
    {
        _alphas[0] = 255;
    }

    inline void concatAlpha(float alpha)
    {
        _alphas[_alphaIdx + 1] = alphaU8Mul(floatToU8(alpha), _alphas[_alphaIdx]);
        _alphaIdx++;
    }

    inline void restoreAlpha()
    {
        _alphaIdx--;
    }

    inline uint8_t curAlpha()
    {
        return _alphas[_alphaIdx];
    }

private:
    uint8_t _alphas[100];
    uint _alphaIdx;
};

#define DBG_RECT(s, r) DBG_LOG("%s %f %f %f %f", s, \
        (r).left(), (r).top(), (r).right(), (r).bottom())

#endif
