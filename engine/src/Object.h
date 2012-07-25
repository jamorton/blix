
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

    virtual ~Object()
    {
        // Stack allocated objects will have a refcount of 1
        // upon destruction.
        DBG_ASSERT(_refcount == 0 || _refcount == 1);
    }

    inline void ref() const { _refcount++; }

    inline void unref() const {
        DBG_ASSERT(_refcount > 0);

        if (--_refcount == 0)
            Engine::destroy(this);
    }

    inline unsigned int refcount() { return _refcount; }

private:
    mutable unsigned int _refcount;
};

template <typename T>
inline T * ref(T * obj)
{
    obj->ref();
    return obj;
}

template <typename T>
inline T * unref(T * obj)
{
    obj->unref();
    return obj;
}

#endif
