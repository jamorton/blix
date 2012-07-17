
#include "EventDispatcher.h"

void EventDispatcher::addEventListener(event_type type, EventDispatcher::EventFunc func)
{
    if (_listeners.count(type) != 0) {
        DBG_LOG("ERROR: addEventListener called on type %d which is already bound", type);
        return;
    }
    _listeners[type] = func;
}

void EventDispatcher::removeEventListener(event_type type)
{
    _listeners.erase(type);
}

void EventDispatcher::dispatchEvent(Event * e)
{
    if (_listeners.count(e->type()) != 0)
        _listeners[e->type()](e);
}
