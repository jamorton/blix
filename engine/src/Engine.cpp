
#include "Engine.h"

#include "skia.h"
#include "SkGraphics.h"
#include "GrContext.h"
#include "SkGpuDevice.h"
#include "GrRenderTarget.h"
#include "SkDevice.h"

#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "gl/GrGLDefines.h"

#include "Stage.h"
#include "Event.h"

extern Stage * createStage();

bool Engine::DRAW_BOUNDS = false;

/*

Precise flash-like click handling would require some kind
of color picking. It is pretty easy with skia, but probably not
necessary given how imprecise touches are on mobile. We probably
just want simple bounds-based click targeting

#include "SkDrawFilter.h"
#include "SkColorFilter.h"
#include "SkUtils.h"

class MyFilter : public SkColorFilter
{
public:
    virtual uint32_t getFlags() SK_OVERRIDE {
        return 0;
    }

    virtual void filterSpan(const SkPMColor shader[], int count,
        SkPMColor result[]) SK_OVERRIDE {
        //sk_memset32(result, SkPreMultiplyColor(0xFF00FF00), count);
        for (uint i = 0; i < count; i++) {

        }
    }
protected:
    SK_DECLARE_UNFLATTENABLE_OBJECT()
};
class MyDrawFilter : public SkDrawFilter
{
public:
    void filter(SkPaint * paint, Type type)
    {
        paint->setColorFilter(&filt);
    }
    MyFilter filt;
};
*/

Engine::Engine()
{

}

void Engine::onLoaded()
{
    _interface = GrGLCreateNativeInterface();
    _context = GrContext::Create(kOpenGL_Shaders_GrEngine, (GrPlatform3DContext)_interface);

    GrPlatformRenderTargetDesc desc;
    desc.fWidth = SkScalarRound(Platform::width());
    desc.fHeight = SkScalarRound(Platform::height());
    desc.fConfig = kSkia8888_PM_GrPixelConfig;
    GR_GL_GetIntegerv(_interface, GR_GL_SAMPLES, &desc.fSampleCnt);
    GR_GL_GetIntegerv(_interface, GR_GL_STENCIL_BITS, &desc.fStencilBits);
    GrGLint buffer;
    GR_GL_GetIntegerv(_interface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    desc.fRenderTargetHandle = buffer;

    _renderTarget = _context->createPlatformRenderTarget(desc);

    SkGpuDevice * dev = new SkGpuDevice(_context, _renderTarget);
    _canvas = new SkCanvas(dev);
    dev->unref();

    _stage = createStage();
}

void Engine::onUnload()
{
    delete _stage;
    SkSafeUnref(_context);
    SkSafeUnref(_interface);
    SkSafeUnref(_renderTarget);
}

void Engine::onUpdate()
{
    uint color = _stage->bgColor();
    glClearColor(
        u8ToFloat((color & 0xFF0000) >> 16),
        u8ToFloat((color & 0x00FF00) >> 8),
        u8ToFloat( color & 0x0000FF),
        1.0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    _stage->update(_canvas);
    _canvas->flush();
}

void Engine::onTouchEvent(TouchType type, float x, float y, uint id)
{
    EventType evt =
        type == TOUCH_DOWN ? TouchEvent::DOWN :
        (type == TOUCH_UP ? TouchEvent::UP
            : TouchEvent::MOVE);

    TouchEvent e(evt, 0, 0, x, y, id);
    SkMatrix m;
    m.reset();
    _stage->_handleTouch(&e, &m);
}

void Engine::onAccelEvent(float x, float y, float z) { }
