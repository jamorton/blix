
#include "Sprite.h"
#include "skia.h"

void Sprite::_draw(Canvas * canvas)
{
    graphics.draw(canvas);
}

void Sprite::_recalcBounds()
{
    if (!_boundsInvalid)
        return;
    DisplayObjectContainer::_recalcBounds();
    _bounds.join(graphics.bounds());
}
