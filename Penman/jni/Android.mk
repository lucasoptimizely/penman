LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
OPENCV_CAMERA_MODULES:=off
OPENCV_MK_PATH:=$(HOME)/Programs/OpenCV4Android/OpenCV-2.4.1/share/opencv/OpenCV.mk
include $(OPENCV_MK_PATH)
LOCAL_MODULE    := svm
LOCAL_SRC_FILES := svm.cpp 
LOCAL_LDLIBS +=  -llog -ldl
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
OPENCV_CAMERA_MODULES:=off
OPENCV_MK_PATH:=$(HOME)/Programs/OpenCV4Android/OpenCV-2.4.1/share/opencv/OpenCV.mk
include $(OPENCV_MK_PATH)
LOCAL_MODULE    := warp
LOCAL_SRC_FILES := warp.cpp 
LOCAL_LDLIBS +=  -llog -ldl
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
OPENCV_CAMERA_MODULES:=off
OPENCV_MK_PATH:=$(HOME)/Programs/OpenCV4Android/OpenCV-2.4.1/share/opencv/OpenCV.mk
include $(OPENCV_MK_PATH)
LOCAL_MODULE    := words
LOCAL_SRC_FILES := words.cpp 
LOCAL_LDLIBS +=  -llog -ldl
include $(BUILD_SHARED_LIBRARY)