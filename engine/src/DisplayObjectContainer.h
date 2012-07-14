
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

    inline size_t numChildren() { return _children.size(); }

    inline bool contains(DisplayObject * child) {
        return child->_parent == this;
    }

    inline size_t getChildIndex(DisplayObject * child) {
        if (!contains(child))
            return 0;
        return child->_containerIndex;
    }


protected:

    void _recalcBounds();

private:

    std::vector<DisplayObject *> _children;

};

#endif
