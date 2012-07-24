
#ifndef ENGINE_OBJECT_H
#define ENGINE_OBJECT_H

#include "Engine.h"

/**
 * Base class for engine and user objects. Provides simple
 * refcounting, custom engine memory management, and a few other
 * conveniences
 */
class Object
{

public:
    Object() : _refcount(1) { }

    virtual ~Object() {
        DBG_ASSERT(_refcount == 0);
    }

    inline void ref() const { _refcount += 1; }

    inline void unref() const {
        DBG_ASSERT(_refcount > 1);

        if (--_refcount == 1)
            Engine::destroy(this);
    }

private:
    mutable unsigned int _refcount;
};

template <typename T>
inline T * ref(T * obj)
{
    obj->ref();
    return obj;
}

#endif
