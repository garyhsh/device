$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

# ext4 filesystem utils
PRODUCT_PACKAGES += \
	e2fsck \
	libext2fs \
	libext2_blkid \
	libext2_uuid \
	libext2_profile \
	libext2_com_err \
	libext2_e2p \
	make_ext4fs \
	hwcomposer.exdroid \
	display.exdroid \
	lights.exdroid \
	camera.exdroid \
	sensors.exdroid 
	
PRODUCT_PACKAGES += \
	audio.primary.exdroid \
	audio.a2dp.default \
	audio.usb.default
	
#softwinner framework add-on api
PRODUCT_PACKAGES += \
	framework2 \
	android.softwinner.framework
	
PRODUCT_PACKAGES += \
	libcedarxbase \
	libcedarxosal \
	libcedarv \
	libcedarv_base \
	libcedarv_adapter \
	libve \
	libaw_audio \
	libaw_audioa \
	libswdrm \
	libstagefright_soft_cedar_h264dec \
	libfacedetection \
	libthirdpartstream \
	libcedarxsftstream \
	libsunxi_alloc \
	libsrec_jni \
	libjpgenc \
	libstagefrighthw \
	libOmxCore \
	libOmxVdec \
	libOmxVenc \
	libaw_h264enc \
	libI420colorconvert \
    libjni_googlepinyinime_latinime_5.so \
    libjni_googlepinyinime_5.so

PRODUCT_PACKAGES += \
  iostat \
  su \
  busybox \
  fsck.exfat \
  mkfs.exfat \
  mkntfs \
  mount.exfat \
  ntfs-3g \
  ntfs-3g.probe

# media
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-common/media_codecs.xml:system/etc/media_codecs.xml \
	device/softwinner/fiber-common/hardware/audio/audio_policy.conf:system/etc/audio_policy.conf

# table core hardware
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-common/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
	frameworks/native/data/etc/android.hardware.faketouch.xml:system/etc/permissions/android.hardware.faketouch.xml

#	FireReader.apk \
# apk
PRODUCT_PACKAGES += \
	4KPlayer.apk \
	flashplayer.apk \
	DragonPhone.apk \
	DragonFire_v2.0.4.apk \
	FileExplore \
	Update.apk \
	Calculator.apk \
	AWGallery \
    	com.google.android.inputmethod.pinyin_403232 \
    	SoftWinnerService.apk \
	Epsxe.apk \
	ARCADE.apk \
	GBA.apk \
	MD.apk \
	N64droid.apk \
	Gbc.apk \
	Sneslite.apk

PRODUCT_PACKAGES += \
  chrome-command-line

#user data backup and resume
#add datacopy.sh for prebuild apk Jyw_allen
PRODUCT_PACKAGES += \
  data_resume.sh \
  preinstall.sh \
  datacopy.sh

#apk lib
PRODUCT_PACKAGES += \
  libebookdec.so \
  libScanFileLib.so \
  libjni_WFDisplay.so \
  libwfdrtsp.so \
  libwfdplayer.so \
  libwfdmanager.so \
  libwfdutils.so \
  libwifidisplay.so \
  libdlna.so \
  libwutils.so \
  libtransfer.so \
  libshairport.so \
  libplist.so \
  libjni_wifidisplay.so \
  libsoftwinner_servers.so
  
# init.rc
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-common/init.rc:root/init.rc \
	device/softwinner/fiber-common/init.sun6i.usb.rc:root/init.sun6i.usb.rc
  
	
# apk data
PRODUCT_COPY_FILES += \
	$(call find-copy-subdir-files,*,$(LOCAL_PATH)/prebuild/apkdata/txt2epub,system/txt2epub) 

#prebuild apk data Jyw_allen
PRODUCT_COPY_FILES += \
	$(call find-copy-subdir-files,*,$(LOCAL_PATH)/prebuild/apkdata/com.epsxe.ePSXe,system/etc/prebuild_data/com.epsxe.ePSXe) \
	$(call find-copy-subdir-files,*,$(LOCAL_PATH)/prebuild/apkdata/com.androidemu.gba,system/etc/prebuild_data/com.androidemu.gba) \
	$(call find-copy-subdir-files,*,$(LOCAL_PATH)/prebuild/apkdata/com.androidemu.gens,system/etc/prebuild_data/com.androidemu.gens) \
	$(call find-copy-subdir-files,*,$(LOCAL_PATH)/prebuild/apkdata/com.tiger.game.arcade2,system/etc/prebuild_data/com.tiger.game.arcade2) \
	$(call find-copy-subdir-files,*,$(LOCAL_PATH)/prebuild/apkdata/com.androidemu.n64,system/etc/prebuild_data/com.androidemu.n64) \
	$(call find-copy-subdir-files,*,$(LOCAL_PATH)/prebuild/apkdata/com.androidemu.gbc,system/etc/prebuild_data/com.androidemu.gbc) \
	$(call find-copy-subdir-files,*,$(LOCAL_PATH)/prebuild/apkdata/com.androidemu.sneslite,system/etc/prebuild_data/com.androidemu.sneslite)

PRODUCT_PROPERTY_OVERRIDES += \
	wifi.interface=wlan0 \
	wifi.supplicant_scan_interval=15 \
	keyguard.no_require_sim=true

PRODUCT_PROPERTY_OVERRIDES += \
	persist.sys.strictmode.visual=0 \
	persist.sys.strictmode.disable=1 \
    ro.reversion.aw_sdk_tag=exdroid4.1.1_r1-a31-v2.0 \
	ro.display.switch=1 \
	ro.eventproc.start=0 \
    ro.wifidisplay.switch=1
	
PRODUCT_PROPERTY_OVERRIDES += \
	dalvik.vm.dexopt-flags=m=y \
	dalvik.vm.dexopt-data-only=1 \

PRODUCT_PROPERTY_OVERRIDES += \
	drm.service.enabled=true
	
PRODUCT_PACKAGES += \
	com.google.widevine.software.drm.xml \
	com.google.widevine.software.drm \
	libdrmwvmplugin \
	libwvm \
	libWVStreamControlAPI_L3 \
	libwvdrm_L3 \
  libdrmdecrypt

#PRODUCT_PROPERTY_OVERRIDES += \
#	dalvik.vm.execution-mode=int:fast

BUILD_NUMBER := $(shell date +%Y%m%d)
