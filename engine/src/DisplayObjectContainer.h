
#ifndef ENGINE_DISPLAY_OBJECT_CONTAINER_H
#define ENGINE_DISPLAY_OBJECT_CONTAINER_H

#include "DisplayObject.h"
#include "common.h"
#include <vector>

class DisplayObjectContainer : public DisplayObject
{

public:

    virtual ~DisplayObjectContainer() { }

    void update(SkCanvas * canvas);

    void addChild(DisplayObject * child);
    void removeChild(DisplayObject * child);

    inline size_t numChildren()  const { return _children.size(); }

    inline bool contains(DisplayObject * child) const {
        return child->_parent == this;
    }

    inline size_t getChildIndex(DisplayObject * child) const {
        if (!contains(child))
            return 0;
        return child->_containerIndex;
    }

protected:

    virtual void _recalcBounds();

private:

    std::vector<DisplayObject *> _children;

    friend class Engine;
    virtual void _handleTouch(TouchEvent * event, SkMatrix * m);

};

#endif
