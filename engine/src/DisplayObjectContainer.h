
#ifndef ENGINE_DISPLAY_OBJECT_CONTAINER_H
#define ENGINE_DISPLAY_OBJECT_CONTAINER_H

#include "DisplayObject.h"
#include "common.h"
#include <vector>

class DisplayObjectContainer : public DisplayObject
{

public:

    virtual ~DisplayObjectContainer();

    void update(Canvas * canvas);

    template <typename T>
    inline T * addChild(T * child)
    {
        child->ref();
        _addChild(child);
        return child;
    }

    template <typename T>
    inline T * giveChild(T * child)
    {
        _addChild(child);
        return child;
    }

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

    void _addChild(DisplayObject * child);

    std::vector<DisplayObject *> _children;
    uint _loopIdx;

    friend class Engine;
    virtual void _handleTouch(TouchEvent * event, SkMatrix * m);

};

#endif
