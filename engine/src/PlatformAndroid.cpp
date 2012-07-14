
#ifdef __ANDROID__

#include <jni.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <ctime>

#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>

#include "common.h"
#include "Platform.h"
#include "IPlatformEventHandler.h"
#include "Engine.h"

static IPlatformEventHandler _dummyHandler;

static struct {
    android_app * app;
    IPlatformEventHandler * handler;
    bool active;

    ASensorEventQueue * sensorEventQueue;
    ASensorManager * sensorManager;
    const ASensor * accelSensor;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    int surfaceWidth;
    int surfaceHeight;

} plat = {NULL, NULL, false, NULL, NULL, NULL, EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_CONTEXT, 0, 0};

int Platform::width()
{
    return plat.surfaceWidth;
}

int Platform::height()
{
    return plat.surfaceHeight;
}

double Platform::getTime()
{
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    return res.tv_sec + (double)res.tv_nsec/1e9;
}

void Platform::readFile(const std::string& path, std::string * out)
{
    if (out == NULL)
        return;

    AAsset * file = AAssetManager_open(plat.app->activity->assetManager, path.c_str(), AASSET_MODE_BUFFER);
    if (file == NULL)
        return;

    out->assign((const char *)AAsset_getBuffer(file), AAsset_getLength(file));
    AAsset_close(file);
}

void Platform::setHandler(IPlatformEventHandler * handler)
{
    plat.handler = handler;
}

static void _setupGraphics()
{
    // initialize OpenGL ES and EGL

    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_DEPTH_SIZE, 0,
        EGL_NONE
    };

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint numConfigs, format;
    EGLConfig config;

    plat.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(plat.display, NULL, NULL);

    eglChooseConfig(plat.display, configAttribs, &config, 1, &numConfigs);

    plat.context = eglCreateContext(plat.display, config, EGL_NO_CONTEXT, contextAttribs);

    eglGetConfigAttrib(plat.display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(plat.app->window, 0, 0, format);

    plat.surface = eglCreateWindowSurface(
        plat.display, config, plat.app->window, NULL);

    if (eglMakeCurrent(plat.display, plat.surface, plat.surface, plat.context) == EGL_FALSE) {
        DBG_LOG("Unable to eglMakeCurrent");
        return;
    }

    eglQuerySurface(plat.display, plat.surface, EGL_WIDTH, &plat.surfaceWidth);
    eglQuerySurface(plat.display, plat.surface, EGL_HEIGHT, &plat.surfaceHeight);
}

static void _destroyGraphics()
{
    if (plat.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(plat.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (plat.context != EGL_NO_CONTEXT) {
            eglDestroyContext(plat.display, plat.context);
        }
        if (plat.surface != EGL_NO_SURFACE) {
            eglDestroySurface(plat.display, plat.surface);
        }
        eglTerminate(plat.display);
    }

    plat.display = EGL_NO_DISPLAY;
    plat.context = EGL_NO_CONTEXT;
    plat.surface = EGL_NO_SURFACE;
}


static void _handleCmd(android_app * app, int32_t cmd)
{
    switch (cmd) {

    case APP_CMD_SAVE_STATE:
        break;

    case APP_CMD_INIT_WINDOW:
        if (plat.app->window != NULL)
            _setupGraphics();
        plat.handler->onLoaded();
        break;

    case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
        plat.handler->onUnload();
        _destroyGraphics();
        break;

    case APP_CMD_GAINED_FOCUS:
        // When our app gains focus, we start monitoring the accelerometer.
        if (plat.accelSensor != NULL) {
            ASensorEventQueue_enableSensor(plat.sensorEventQueue,
                                           plat.accelSensor);
            // We'd like to get 60 events per second (in us).
            ASensorEventQueue_setEventRate(plat.sensorEventQueue,
                                           plat.accelSensor, (1000L/60)*1000);
        }
        plat.active = true;
        break;

    case APP_CMD_LOST_FOCUS:
        // When our app loses focus, we stop monitoring the accelerometer.
        // This is to avoid consuming battery while not being used.
        if (plat.accelSensor != NULL) {
            ASensorEventQueue_disableSensor(plat.sensorEventQueue,
                                            plat.accelSensor);
        }
        plat.active = false;
        break;
    }
}

// TODO: Support multi-touch
static int32_t _handleInput(android_app * app, AInputEvent * event)
{
    if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION)
        return 0;

    PlatformTouchEvent ev;

    ev.x = AMotionEvent_getX(event, 0);
    ev.y = AMotionEvent_getY(event, 0);

    int index = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK;
    ev.id = AMotionEvent_getPointerId(event, index);

    switch (AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK) {

    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
        ev.type = PlatformTouchEvent::ACTION_DOWN;
        break;

    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP:
        ev.type = PlatformTouchEvent::ACTION_UP;
        break;

    case AMOTION_EVENT_ACTION_MOVE:
        ev.type = PlatformTouchEvent::ACTION_MOVE;
        break;

    }

    plat.handler->onTouchEvent(ev);

    return 0;
}

static void _eventLoop()
{
    double loopStartTime = 0.0, loopEndTime = 0.0;
    // TODO: configurable
    double secondsPerFrame = 1/30.0;


    srand(time(NULL));

    while (1) {

        // Maintain a maximum framerate (important on mobile devices to save battery)
        loopEndTime = Platform::getTime();
        double dt = loopEndTime - loopStartTime;
        if (dt < secondsPerFrame)
            usleep((useconds_t)((secondsPerFrame - dt) * 1e6));
        loopStartTime = Platform::getTime();

        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If activity isn't active (forground), block for events (won't exit this loop and animate)
        while ((ident=ALooper_pollAll(plat.active ? 0 : -1, NULL, &events,
                    (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(plat.app, source);
            }

            // Process any sensor data
            if (ident == LOOPER_ID_USER) {
                if (plat.accelSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(plat.sensorEventQueue,
                            &event, 1) > 0) {

                        plat.handler->onAccelData(
                            PlatformAccelData(
                                event.acceleration.x,
                                event.acceleration.y,
                                event.acceleration.z
                            )
                        );
                    }
                }
            }

            // Check if we are exiting.
            if (plat.app->destroyRequested != 0) {
                _destroyGraphics();
                return;
            }
        }


        // Frame update event
        if (plat.display != EGL_NO_DISPLAY) {
            plat.handler->onUpdate();
            eglSwapBuffers(plat.display, plat.surface);
        }
    }
}

extern "C"
void android_main(struct android_app* app) {

    // Make sure glue isn't stripped.
    app_dummy();

    plat.app = app;
    plat.handler = &_dummyHandler;
    plat.active = false;

    app->onAppCmd = _handleCmd;
    app->onInputEvent = _handleInput;

    plat.sensorManager = ASensorManager_getInstance();
    plat.accelSensor = ASensorManager_getDefaultSensor(plat.sensorManager,
        ASENSOR_TYPE_ACCELEROMETER);
    plat.sensorEventQueue = ASensorManager_createEventQueue(plat.sensorManager,
        app->looper, LOOPER_ID_USER, NULL, NULL);

    plat.display = EGL_NO_DISPLAY;
    plat.context = EGL_NO_CONTEXT;
    plat.surface = EGL_NO_SURFACE;

    Engine e;
    Platform::setHandler(&e);

    _eventLoop();
}

// TODO: Better event handling, as in
// https://github.com/blackberry/GamePlay/blob/master/gameplay/src/PlatformAndroid.cpp


#endif
