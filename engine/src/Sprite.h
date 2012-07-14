
#ifndef ENGINE_SPRITE_H
#define ENGINE_SPRITE_H

#include "DisplayObjectContainer.h"
#include "Graphics.h"

class SkCanvas;

class Sprite : public DisplayObjectContainer
{

public:

    Sprite() { }
    virtual ~Sprite() { }

    Graphics graphics;

protected:

    void _draw(SkCanvas * canvas);

private:

};

#endif
