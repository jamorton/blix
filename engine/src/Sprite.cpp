
#include "Sprite.h"
#include "skia.h"

void Sprite::_draw(SkCanvas * canvas)
{
    graphics.draw(canvas);
}

void Sprite::_recalcBounds()
{
    DisplayObjectContainer::_recalcBounds();
    _bounds.join(graphics.bounds());
}
