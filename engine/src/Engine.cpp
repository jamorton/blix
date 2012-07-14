
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

extern Stage * createStage();

bool Engine::DRAW_BOUNDS = false;

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
    SkSafeUnref(_context);
    SkSafeUnref(_interface);
    SkSafeUnref(_renderTarget);

    delete _stage;
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

void Engine::onTouchEvent(PlatformTouchEvent event) { }
void Engine::onAccelData(PlatformAccelData event) { }
