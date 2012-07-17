
#ifndef ENGINE_EVENT_H
#define ENGINE_EVENT_H

typedef int EventType;

#define DEFINE_EVENT_TYPE(e) enum { e = __COUNTER__ }

class Event
{
public:
    Event(EventType t) : _type(t) { }

    inline EventType type() { return _type; }

    template <typename T>
    inline T * event() { return static_cast<T *>(this); }

    template <typename T>
    inline T * target() { return reinterpret_cast<T *>(_target); }

private:
    EventType _type;
    void * _target;
    friend class EventDispatcher;

public:

    DEFINE_EVENT_TYPE( ENTER_FRAME );

};



#endif
