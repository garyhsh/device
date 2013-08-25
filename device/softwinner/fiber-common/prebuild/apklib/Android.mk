# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libebookdec.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libScanFileLib.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libjni_WFDisplay.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libwfdrtsp.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libwfdplayer.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libwfdmanager.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libwfdutils.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libwifidisplay.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libdlna.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libwutils.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libtransfer.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libshairport.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libplist.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libjni_wifidisplay.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libjni_googlepinyinime_latinime_5.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libjni_googlepinyinime_5.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng optional

LOCAL_PREBUILT_LIBS := libcheckfile.so 
				

include $(BUILD_MULTI_PREBUILT)

