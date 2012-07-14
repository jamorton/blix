
#ifndef IPLATFORM_EVENT_HANDLER_H
#define IPLATFORM_EVENT_HANDLER_H

#include "common.h"

typedef uintptr_t platform_touch_id;

/**
 * Multitouch events
 */
struct PlatformTouchEvent
{
    enum {
        ACTION_DOWN,
        ACTION_MOVE,
        ACTION_UP
    } type;

    float x;
    float y;

    platform_touch_id id;
};

/**
 * Accelerometer data event
 * TODO: just use a vector class for this?
 */
struct PlatformAccelData
{
    PlatformAccelData(float x_, float y_, float z_) : x(x_), y(y_), z(z_) { }

    float x;
    float y;
    float z;
};

/**
 * Implement this interface to handle platform specific events
 */
class IPlatformEventHandler
{

public:

    virtual void onUpdate() { }

    // Called after game has been loaded
    virtual void onLoaded() { }
    // Called before game is unloaded
    virtual void onUnload() { }

    virtual void onTouchEvent(PlatformTouchEvent event) { }
    virtual void onAccelData(PlatformAccelData event) { }
};

#endif
