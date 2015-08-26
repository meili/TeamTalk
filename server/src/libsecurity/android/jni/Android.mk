LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := ../include
LOCAL_MODULE    := security
LOCAL_SRC_FILES := aes_core.cpp base64.cpp md5.cpp security.cpp
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_CFLAGS := -D__ANDROID__
include $(BUILD_SHARED_LIBRARY)
