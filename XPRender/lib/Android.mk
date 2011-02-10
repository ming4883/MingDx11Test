LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE	:= xprender


#LOCAL_SRC_FILES	+= xprender/Buffer.c
#LOCAL_SRC_FILES	+= xprender/GpuState.c
#LOCAL_SRC_FILES	+= xprender/Mat44.c
#LOCAL_SRC_FILES	+= xprender/NvpParser.c
#LOCAL_SRC_FILES	+= xprender/Platform.c
#LOCAL_SRC_FILES	+= xprender/RenderTarget.c
#LOCAL_SRC_FILES	+= xprender/Shader.c
#LOCAL_SRC_FILES	+= xprender/StrHash.c
#LOCAL_SRC_FILES	+= xprender/Texture.c
#LOCAL_SRC_FILES	+= xprender/Vec2.c
#LOCAL_SRC_FILES	+= xprender/Vec3.c
#LOCAL_SRC_FILES	+= xprender/Vec4.c

LOCAL_SRC_FILES	+= pez/pez.android.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/xprender $(LOCAL_PATH)/pez
LOCAL_CFLAGS := -DXPR_ANDROID

LOCAL_STATIC_LIBRARIES := android_native_app_glue

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)
LOCAL_EXPORT_LDLIBS := -llog -landroid -lEGL -lGLESv1_CM

include $(BUILD_STATIC_LIBRARY)
