
OBJ_PATH := $(ENGINE_PATH)/libs/android

LOCAL_PATH := $(OBJ_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := libengine
LOCAL_SRC_FILES := libengine.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpng
LOCAL_SRC_FILES := libpng.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libfreetype
LOCAL_SRC_FILES := libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libskia
LOCAL_SRC_FILES := libskia.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_PATH := $(GAME_JNI_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE := $(GAME_MODULE_NAME)
LOCAL_SRC_FILES := $(GAME_SOURCES)
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2 -lOpenSLES -lz
LOCAL_STATIC_LIBRARIES := libengine android_native_app_glue libskia libpng libfreetype
LOCAL_CFLAGS := -O2 -std=gnu++0x
LOCAL_C_INCLUDES := $(ENGINE_PATH)/engine/src $(ENGINE_PATH)/deps/skia/include/core \
                    $(ENGINE_PATH)/deps/skia/include/config

include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/native_app_glue)
