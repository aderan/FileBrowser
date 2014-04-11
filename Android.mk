LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE    := glib
LOCAL_SRC_FILES := \
$(LOCAL_PATH)/../glib-airplay/obj/local/armeabi/libglib-2.0.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := shairplay
LOCAL_SRC_FILES := \
$(LOCAL_PATH)/../common/lib/libshairplay.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := xml2
LOCAL_SRC_FILES := \
$(LOCAL_PATH)/../libxml2-airplay/obj/local/armeabi/libxml2.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := icui18n
LOCAL_SRC_FILES := \
$(LOCAL_PATH)/../common/lib/libicui18n.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := icuuc
LOCAL_SRC_FILES := \
$(LOCAL_PATH)/../common/lib/libicuuc.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE     := airplay_jni
LOCAL_SRC_FILES  := airplay_jni.cpp json.c kairnet.cpp kairplay.cpp kairport.cpp 

LOCAL_LDLIBS     += -llog
LOCAL_LDLIBS     += -landroid

LOCAL_STATIC_LIBRARIES     += shairplay glib xml2
LOCAL_SHARED_LIBRARIES     += icui18n icuuc

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../glib-airplay/glib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../glib-airplay/android
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../glib-airplay

#LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

include $(BUILD_SHARED_LIBRARY)
