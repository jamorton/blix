
JNI_PATH := $(call my-dir)
DEPS_PATH := ../../deps

include $(JNI_PATH)/common.mk
include $(JNI_PATH)/libpng.mk
include $(JNI_PATH)/freetype.mk
include $(JNI_PATH)/skia.mk

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

TARGET_PLATFORM := android-9

LOCAL_MODULE    := libengine
LOCAL_SRC_FILES := $(addprefix ../,$(wildcard $(JNI_PATH)/../../src/*.cpp))
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -lz -lOpenSLES
LOCAL_STATIC_LIBRARIES := android_native_app_glue libskia libpng libfreetype
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../engine $(COMMON_INCLUDES)

LOCAL_CFLAGS := -O2 -std=gnu++0x

include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/native_app_glue)
