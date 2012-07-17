
#include "DisplayObject.h"
#include "DisplayObjectContainer.h"
#include "Engine.h"
#include "skia.h"

void DisplayObject::update(SkCanvas * canvas)
{
    Event e(Event::ENTER_FRAME);
    dispatchEvent(&e);

    canvas->save();
    _recalcBounds();
    canvas->concat(_transform);

    _draw(canvas);

    if (Engine::DRAW_BOUNDS)
        _drawBounds(canvas);

    canvas->restore();
}


void DisplayObject::_drawBounds(SkCanvas * canvas)
{
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

void DisplayObject::_recalcBounds()
{
    if (!_boundsInvalid)
        return;
    _transform.reset();
    _transform.preTranslate(x, y);
    _transform.preScale(scaleX, scaleY);
    _transform.preRotate(rotation);
}
