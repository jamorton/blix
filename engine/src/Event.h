
#ifndef ENGINE_EVENT_H
#define ENGINE_EVENT_H

typedef int event_type;

#define DEFINE_EVENT_TYPE(e) enum { e = __COUNTER__ }

class Event
{
public:
    Event(event_type t) : _type(t) { }

    inline event_type type() { return _type; }

    template <typename T>
    inline T * get() { return static_cast<T *>(this); }

private:

    event_type _type;
};



#endif
