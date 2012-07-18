
GAME_MODULE_NAME := BlixTests
GAME_JNI_PATH := $(call my-dir)
ENGINE_PATH := $(GAME_JNI_PATH)/../../..

GAME_SOURCES := $(addprefix ../,$(wildcard $(GAME_JNI_PATH)/../../src/*.cpp))

include $(ENGINE_PATH)/tools/game.mk
