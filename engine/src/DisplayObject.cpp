
#include "DisplayObject.h"
#include "DisplayObjectContainer.h"
#include "Engine.h"
#include "skia.h"

void DisplayObject::update(SkCanvas * canvas)
{
    canvas->save();
    _applyTransform(canvas);

    _draw(canvas);

    if (Engine::DRAW_BOUNDS)
        _drawBounds(canvas);

    canvas->restore();
}

void DisplayObject::_applyTransform(SkCanvas * canvas)
{
    canvas->translate(x, y);
    canvas->scale(scaleX, scaleY);
    canvas->rotate(rotation);
}

void DisplayObject::_drawBounds(SkCanvas * canvas)
{
    _recalcBounds();
    SkPaint p;
    p.setARGB(255, 255, 0, 0);
    p.setStrokeWidth(1);
    p.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(_bounds, p);
}

void DisplayObject::_invalidateBounds()
{
    if (_boundsInvalid)
        return;
    _boundsInvalid = true;
    if (_parent != NULL)
        _parent->_invalidateBounds();
}
