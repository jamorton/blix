
#include "DisplayObjectContainer.h"
#include "Engine.h"
#include "skia.h"

void DisplayObjectContainer::update(SkCanvas * canvas)
{
    canvas->save();
    _applyTransform(canvas);

    _draw(canvas);

    for (int i = 0; i < _children.size(); i++)
        _children[i]->update(canvas);

    if (Engine::DRAW_BOUNDS)
        _drawBounds(canvas);

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
    _bounds.setEmpty();
    // a DOC's bounds is the  union of its children's bounds
    for (int i = 0; i < _children.size(); i++) {
        DisplayObject * child = _children[i];
        child->_recalcBounds();
        SkRect bound = child->_bounds;
        bound.offset(child->x, child->y);
        _bounds.join(bound);
    }
}
