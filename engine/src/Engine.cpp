
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

#include "Platform.h"
#include "Stage.h"
#include "Event.h"
#include "Object.h"

extern Stage * createStage();

bool Engine::DRAW_BOUNDS = false;
bool Engine::_updating = false;
std::vector<const Object *> Engine::_collect;

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
    _canvas = new Canvas(dev);
    dev->unref();

    _stage = ref(createStage());
}

void Engine::onUnload()
{
    _stage->unref();
    _context->unref();
    _interface->unref();
    _renderTarget->unref();
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

    _updating = true;
    _stage->update(_canvas);
    _canvas->flush();
    _updating = false;

    for (int i = 0; i < _collect.size(); i++)
        delete _collect[i];
    _collect.clear();
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
    _updating = true;
    _stage->_handleTouch(&e, &m);
    _updating = false;
}

void Engine::onAccelEvent(float x, float y, float z) { }

void Engine::destroy(const Object * o)
{
    // Problems arise if objects in the display tree are deleted
    // while a frame update is in process, and this is usually where
    // it happens since user code is called from there. We have to
    // defer the delete until after the update is done.
    if (_updating)
        _collect.push_back(o);
    else
        delete o;
}
