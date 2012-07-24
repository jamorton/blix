
#ifndef IPLATFORM_EVENT_HANDLER_H
#define IPLATFORM_EVENT_HANDLER_H

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

    enum TouchType {
        TOUCH_DOWN,
        TOUCH_UP,
        TOUCH_MOVE
    };
    virtual void onTouchEvent(TouchType type, float x, float y, unsigned int id) { }
    virtual void onAccelEvent(float x, float y, float z) { }
};

#endif
