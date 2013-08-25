# Copyright 2006 The Android Open Source Project

# Setting LOCAL_PATH will mess up all-subdir-makefiles, so do it beforehand.
LOCAL_PATH:= $(call my-dir)


include $(addsuffix /Android.mk, $(addprefix $(LOCAL_PATH)/, \
			hwcomposer \
			display \
			libsensor_bma250 \
			libsensors_common \
			lights \
			gps \
			))
