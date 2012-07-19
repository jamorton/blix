
#ifndef ENGINE_SPRITE_H
#define ENGINE_SPRITE_H

#include "common.h"
#include "DisplayObjectContainer.h"
#include "Graphics.h"

class Sprite : public DisplayObjectContainer
{

public:

    Sprite() : graphics(memberFunc(Sprite::_invalidateBounds)) { }
    virtual ~Sprite() { }

    Graphics graphics;

protected:

    void _recalcBounds();
    void _draw(Canvas * canvas);

private:

};

#endif
