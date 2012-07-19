
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

    for (int i = 0; i < _children.size(); i++)
        _children[i]->update(canvas);

    if (Engine::DRAW_BOUNDS)
        _drawBounds(canvas);

    canvas->restoreAlpha();
    canvas->restore();
}

void DisplayObjectContainer::addChild(DisplayObject * child)
{
    if (child->_parent != NULL)
        child->_parent->removeChild(child);
    _children.push_back(child);
    child->_parent = this;
    child->_containerIndex = _children.size() - 1;
    _invalidateBounds();
}

void DisplayObjectContainer::removeChild(DisplayObject * child)
{
    if (!contains(child))
        return;
    _children.erase(_children.begin() + child->_containerIndex);
    child->_parent = NULL;
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

    // find the first child (from front of display list back)
    // that has bounds the touch point are contained in.
    for (int i = _children.size() - 1; i >= 0; i--) {
        DisplayObject * child = _children[i];

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
    dispatchEvent(event);
}
