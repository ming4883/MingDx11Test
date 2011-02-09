LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= xprender
LOCAL_SRC_FILES:= \
	xprender\Buffer.c \
	xprender\GpuState.c \
	xprender\Mat44.c \
	
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_EXPORT_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
