
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <functional>

#include "common.h"
#include "Event.h"

class EventDispatcher
{
public:

    virtual ~EventDispatcher() { }

    typedef std::function<void (Event *)> EventFunc;

    void addEventListener(event_type type, EventFunc evt);
    void removeEventListener(event_type type);

    void dispatchEvent(Event * e);

private:

    hash_table<event_type, EventFunc> _listeners;

};


#endif
