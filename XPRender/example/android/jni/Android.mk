LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := xprender-example

LOCAL_SRC_FILES += ../../Android.example.c

LOCAL_STATIC_LIBRARIES := xprender android_native_app_glue
#LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv1_CM

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
$(call import-module, lib)
