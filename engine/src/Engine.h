
#ifndef ENGINE_H
#define ENGINE_H

#include <vector>

#include "IPlatformEventHandler.h"

class GrContext;
class GrGLInterface;
class GrRenderTarget;
class Canvas;

class Stage;
class Object;

class Engine : public IPlatformEventHandler
{

public:

    Engine();

    void onUpdate();

    void onLoaded();
    void onUnload();

    void onTouchEvent(TouchType type, float x, float y, unsigned int id);
    void onAccelEvent(float x, float y, float z);

    static bool DRAW_BOUNDS;

    static void destroy(const Object * o);

private:

    GrContext * _context;
    const GrGLInterface * _interface;
    GrRenderTarget * _renderTarget;
    Canvas * _canvas;
    Stage * _stage;

    static bool _updating;
    static std::vector<const Object *> _collect;

};

#endif
