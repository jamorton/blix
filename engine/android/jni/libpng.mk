
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SOURCES :=     \
	png.c      \
	pngerror.c \
	pngget.c   \
	pngmem.c   \
	pngpread.c \
	pngread.c  \
	pngrio.c   \
	pngrtran.c \
	pngrutil.c \
	pngset.c   \
	pngtrans.c \
	pngwio.c   \
	pngwrite.c \
	pngwtran.c \
	pngwutil.c

LOCAL_SRC_FILES := $(addprefix ../$(DEPS_PATH)/libpng/,$(SOURCES))
LOCAL_MODULE    := libpng
LOCAL_LDLIBS    := -lz
LOCAL_CFLAGS    := -O2

include $(BUILD_STATIC_LIBRARY)
