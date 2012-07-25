
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <functional>

#include "common.h"
#include "Event.h"
#include "Object.h"

/**
 * A simple flash-like event system, with a few limitations:
 *  - No capture stage
 *  - An object can only have one listener of each event type
 *    (because std::function objects aren't comparable)
 */
class EventDispatcher : public Object
{
public:

    virtual ~EventDispatcher() { }

    typedef std::function<void (Event *)> EventFunc;

    void addEventListener(EventType type, EventFunc evt);
    void removeEventListener(EventType type);

    void dispatchEvent(Event * e);

private:

    hash_table<EventType, EventFunc> _listeners;

};


#endif
