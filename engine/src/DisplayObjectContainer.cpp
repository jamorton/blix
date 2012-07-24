
#include "DisplayObjectContainer.h"
#include "Engine.h"
#include "skia.h"

void DisplayObjectContainer::update(Canvas * canvas)
{
    _recalcBounds();

    Event e(Event::ENTER_FRAME);
    dispatchEvent(&e);

    if (!visible)
        return;

    canvas->save();
    canvas->concat(_transform);
    canvas->concatAlpha(alpha);

    _draw(canvas);

    for (_loopIdx = 0; _loopIdx < _children.size(); _loopIdx++) {
        _children[_loopIdx]->_containerIndex = _loopIdx;
        _children[_loopIdx]->update(canvas);
    }

    if (Engine::DRAW_BOUNDS)
        _drawBounds(canvas);

    canvas->restoreAlpha();
    canvas->restore();
}

void DisplayObjectContainer::addChild(DisplayObject * child)
{
    child->ref();
    if (child->_parent != NULL)
        child->_parent->removeChild(child);
    child->_containerIndex = _children.size();
    _children.push_back(child);
    child->_parent = this;
    _invalidateBounds();
}

void DisplayObjectContainer::removeChild(DisplayObject * child)
{
    if (!contains(child))
        return;
    _children.erase(_children.begin() + child->_containerIndex);
    if (child->_containerIndex <= _loopIdx)
        _loopIdx--;
    child->_parent = NULL;
    child->unref();
    _invalidateBounds();
}

void DisplayObjectContainer::_recalcBounds()
{
    if (!_boundsInvalid)
        return;
    DisplayObject::_recalcBounds();
    _bounds.setEmpty();
    // a DOC's bounds is the  union of its children's bounds
    for (int i = 0; i < _children.size(); i++) {
        DisplayObject * child = _children[i];
        SkRect bound;
        child->_transform.mapRect(&bound, child->bounds());
        _bounds.join(bound);
    }
}

// TODO: a DOC's _handleTouch basically implements globalToLocal.
// should extract that out to a separate method and cache the results
void DisplayObjectContainer::_handleTouch(TouchEvent * event, SkMatrix * m)
{
    SkMatrix trans;
    SkPoint pt;

    event->_target = this;

    // find the first child (from front of display list back)
    // that has bounds the touch point are contained in.
    for (_loopIdx = 0; _loopIdx < _children.size(); _loopIdx++) {
        DisplayObject * child = _children[_loopIdx];

        child->_inverseTransform(trans);
        trans.setConcat(trans, *m);

        pt.set(event->stageX(), event->stageY());
        trans.mapPoints(&pt, &pt, 1);

        if (child->bounds().contains(pt)) {
            *m = trans;
            // these will get overwritten until we reach a display object
            // and the local coords will be correct then
            event->_localX = pt.x();
            event->_localY = pt.y();
            child->_handleTouch(event, m);
            break;
        }
    }
    event->_currentTarget = this;
    dispatchEvent(event);
}
