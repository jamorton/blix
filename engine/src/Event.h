
#ifndef ENGINE_EVENT_H
#define ENGINE_EVENT_H

typedef int EventType;

#define DEFINE_EVENT_TYPE(e) static const EventType e = __COUNTER__

class Event
{
public:
    Event(EventType t) : _type(t), _target(NULL) { }

    inline EventType type() { return _type; }

    template <typename T>
    inline T * event() { return static_cast<T *>(this); }

    template <typename T>
    inline T * target() { return reinterpret_cast<T *>(_target); }

    inline void target(void * t) { _target = t; }

private:
    EventType _type;
    void * _target;
    friend class EventDispatcher;

public:

    DEFINE_EVENT_TYPE( ENTER_FRAME );

};

class TouchEvent : public Event
{
public:
    TouchEvent(EventType t, float x, float y, uint id) :
        Event(t), _x(x), _y(y), _id(id) { }

    inline float x() { return _x; }
    inline float y() { return _y; }
    inline uint id() { return _id; }

private:

    float _x, _y;
    uint _id;

public:

    DEFINE_EVENT_TYPE( DOWN );
    DEFINE_EVENT_TYPE( MOVE );
    DEFINE_EVENT_TYPE( UP );
};


#endif
