
#ifndef ENGINE_H
#define ENGINE_H

#include "common.h"
#include "platform.h"
#include "IPlatformEventHandler.h"

class GrContext;
class GrGLInterface;
class GrRenderTarget;
class SkCanvas;

class DisplayObject;
class Stage;

class Engine : public IPlatformEventHandler
{

public:

    Engine();

    void onUpdate();

    void onLoaded();
    void onUnload();

    void onTouchEvent(PlatformTouchEvent event);
    void onAccelData(PlatformAccelData event);

    static bool DRAW_BOUNDS;

private:

    GrContext * _context;
    const GrGLInterface * _interface;
    GrRenderTarget * _renderTarget;
    SkCanvas * _canvas;
    Stage * _stage;

};

#endif
