
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
    // this is a good place to update the transform cache.
    // some things that invalidate the bounds don't invalidate the
    // transform, but it doens't seem worth it to have whole separate
    // _transformInvalid system
    _transform.reset();
    _transform.preTranslate(x - regX, y - regY);
    _transform.preScale(scaleX, scaleY, regX, regY);
    // if we call rotation functions on matrices they disable certain fast paths
    // the common case is that rotation is 0
    if (rotation != 0)
        _transform.preRotate(rotation, regX, regY);
    _boundsInvalid = false;
}

void DisplayObject::_inverseTransform(SkMatrix& m)
{
    m.setRotate(-rotation, regX, regY);
    m.preScale( 1 / scaleX, 1 / scaleY, regX, regY);
    m.preTranslate(-(x - regX), -(y - regY));
}

void DisplayObject::_handleTouch(TouchEvent * event, SkMatrix * m)
{
    event->target(this);
    dispatchEvent(event);
}
