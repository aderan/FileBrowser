LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := fb_jni
LOCAL_SRC_FILES  := fb_jni.c json.c fb_file.c fb_utils.c

LOCAL_LDLIBS     += -llog
LOCAL_LDLIBS     += -L/home/aderan/toolchains/android-14/sysroot/usr/lib/ -landroid

#LOCAL_LDLIBS     += -landroid

include $(BUILD_SHARED_LIBRARY)
