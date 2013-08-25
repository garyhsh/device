$(call inherit-product, device/softwinner/fiber-common/fiber-common.mk)
$(call inherit-product, frameworks/native/build/tablet-7in-hdpi-1024-dalvik-heap.mk)

# init.rc, kernel
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-hklt/kernel:kernel \
	device/softwinner/fiber-hklt/init.sun6i.rc:root/init.sun6i.rc \
	device/softwinner/fiber-hklt/ueventd.sun6i.rc:root/ueventd.sun6i.rc \
	device/softwinner/fiber-hklt/modules/modules/nand.ko:root/nand.ko \
	device/softwinner/fiber-hklt/initlogo.rle:root/initlogo.rle \
	device/softwinner/fiber-hklt/needfix.rle:root/needfix.rle \
	device/softwinner/fiber-hklt/media/bootanimation.zip:system/media/bootanimation.zip \
	device/softwinner/fiber-hklt/media/boot.wav:system/media/boot.wav


#key and tp config file
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-hklt/sw-keyboard.kl:system/usr/keylayout/sw-keyboard.kl \
	device/softwinner/fiber-hklt/ft5x_ts.idc:system/usr/idc/ft5x_ts.idc \
	device/softwinner/fiber-hklt/gsensor.cfg:system/usr/gsensor.cfg  \
    device/softwinner/fiber-hklt/eio_kp.kl:system/usr/keylayout/eio_kp.kl \
    device/softwinner/fiber-hklt/eio_kp.idc:system/usr/idc/eio_kp.idc

#copy touch and keyboard driver to recovery randisk
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-hklt/modules/modules/ft5x_ts.ko:obj/touch.ko \
  device/softwinner/fiber-hklt/modules/modules/sw-keyboard.ko:obj/keyboard.ko\
  device/softwinner/fiber-hklt/modules/modules/disp.ko:obj/disp.ko\
  device/softwinner/fiber-hklt/modules/modules/lcd.ko:obj/lcd.ko\
  device/softwinner/fiber-hklt/modules/modules/hdmi.ko:obj/hdmi.ko

# wifi & bt config file
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    system/bluetooth/data/main.nonsmartphone.conf:system/etc/bluetooth/main.conf

# rtl8723as bt fw and config
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-common/hardware/realtek/bluetooth/rtl8723as/rlt8723a_chip_b_cut_bt40_fw.bin:system/etc/rlt8723a_chip_b_cut_bt40_fw.bin \
	device/softwinner/fiber-common/hardware/realtek/bluetooth/rtl8723as/rtk8723_bt_config:system/etc/rtk8723_bt_config

# LiveWallpaper
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml  
# usb accessory
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml  

#vold config
PRODUCT_COPY_FILES += \
   device/softwinner/fiber-hklt/vold.fstab:system/etc/vold.fstab \
   device/softwinner/fiber-hklt/recovery.fstab:recovery.fstab

# 3G Data Card Packages
PRODUCT_PACKAGES += \
	u3gmonitor \
	chat \
	rild

# 3G Data Card Configuration Flie
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-common/rild/ip-down:system/etc/ppp/ip-down \
	device/softwinner/fiber-common/rild/ip-up:system/etc/ppp/ip-up \
	device/softwinner/fiber-common/rild/call-pppd:system/etc/ppp/call-pppd \
	device/softwinner/fiber-common/rild/3g_dongle.cfg:system/etc/3g_dongle.cfg \
	device/softwinner/fiber-common/rild/usb_modeswitch:system/bin/usb_modeswitch \
	device/softwinner/fiber-common/rild/usb_modeswitch.sh:system/etc/usb_modeswitch.sh \
	device/softwinner/fiber-common/rild/apns-conf_sdk.xml:system/etc/apns-conf.xml \
	device/softwinner/fiber-common/rild/libsoftwinner-ril.so:system/lib/libsoftwinner-ril.so

# 3G Data Card usb modeswitch File
PRODUCT_COPY_FILES += \
	$(call find-copy-subdir-files,*,device/softwinner/fiber-common/rild/usb_modeswitch.d,system/etc/usb_modeswitch.d)

# camera
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-hklt/camera.cfg:system/etc/camera.cfg \
	device/softwinner/fiber-hklt/media_profiles.xml:system/etc/media_profiles.xml \
	frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
	frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
	frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml
	
#egl
PRODUCT_COPY_FILES += \
       device/softwinner/fiber-common/egl/pvrsrvctl:system/vendor/bin/pvrsrvctl \
       device/softwinner/fiber-common/egl/libusc.so:system/vendor/lib/libusc.so \
       device/softwinner/fiber-common/egl/libglslcompiler.so:system/vendor/lib/libglslcompiler.so \
       device/softwinner/fiber-common/egl/libIMGegl.so:system/vendor/lib/libIMGegl.so \
       device/softwinner/fiber-common/egl/libpvr2d.so:system/vendor/lib/libpvr2d.so \
       device/softwinner/fiber-common/egl/libpvrANDROID_WSEGL.so:system/vendor/lib/libpvrANDROID_WSEGL.so \
       device/softwinner/fiber-common/egl/libPVRScopeServices.so:system/vendor/lib/libPVRScopeServices.so \
       device/softwinner/fiber-common/egl/libsrv_init.so:system/vendor/lib/libsrv_init.so \
       device/softwinner/fiber-common/egl/libsrv_um.so:system/vendor/lib/libsrv_um.so \
       device/softwinner/fiber-common/egl/libEGL_POWERVR_SGX544_115.so:system/vendor/lib/egl/libEGL_POWERVR_SGX544_115.so \
       device/softwinner/fiber-common/egl/libGLESv1_CM_POWERVR_SGX544_115.so:system/vendor/lib/egl/libGLESv1_CM_POWERVR_SGX544_115.so \
       device/softwinner/fiber-common/egl/libGLESv2_POWERVR_SGX544_115.so:system/vendor/lib/egl/libGLESv2_POWERVR_SGX544_115.so \
       device/softwinner/fiber-common/egl/gralloc.sun6i.so:system/vendor/lib/hw/gralloc.sun6i.so \
       device/softwinner/fiber-common/egl/egl.cfg:system/lib/egl/egl.cfg \
       device/softwinner/fiber-common/egl/powervr.ini:system/etc/powervr.ini

#rmvb  
PRODUCT_COPY_FILES += \
       device/softwinner/fiber-common/CedarX-Crack/libdemux_rmvb.so:system/lib/libdemux_rmvb.so \
       device/softwinner/fiber-common/CedarX-Crack/librm.so:system/lib/librm.so \
       device/softwinner/fiber-common/CedarX-Crack/libswa1.so:system/lib/libswa1.so \
       device/softwinner/fiber-common/CedarX-Crack/libswa2.so:system/lib/libswa2.so \
       
# facelock
PRODUCT_PACKAGES += \
	FaceLock.apk \
	libfacelock_jni.so

# Jyw_Cjp add for debug_uesd apk
PRODUCT_COPY_FILES += \
	$(call find-copy-subdir-files,*.apk,$(LOCAL_PATH)/debug_apk,system/app) 

# Jyw_Cjp add for pre_install apk
PRODUCT_COPY_FILES += \
    $(call find-copy-subdir-files,*.apk,$(LOCAL_PATH)/pre_apk/apk,system/app) \
    $(call find-copy-subdir-files,*.so,$(LOCAL_PATH)/pre_apk/lib,system/lib)


#	device/softwinner/fiber-hklt/emulator/lib/N64/libsound.so:system/lib/N64/libsound.so \
#	device/softwinner/fiber-hklt/emulator/lib/N64/libn64.so:system/lib/N64/libn64.so \
#	device/softwinner/fiber-hklt/emulator/lib/N64/libn64jni.so:system/lib/N64/libn64jni.so \
# Jyw_Cjp add for emulator
PRODUCT_COPY_FILES += \
   	$(call find-copy-subdir-files,*.apk,$(LOCAL_PATH)/emulator/apk,system/app) \
	device/softwinner/fiber-hklt/emulator/lib/Nes4droid/libemu.so:system/lib/Nes4droid/libemu.so \
	device/softwinner/fiber-hklt/emulator/lib/Nes4droid/libnes.so:system/lib/Nes4droid/libnes.so 

#PRODUCT_PROPERTY_OVERRIDES += \
#	persist.sys.timezone=Asia/Shanghai \
#	persist.sys.language=zh \
#	persist.sys.country=CN

PRODUCT_PROPERTY_OVERRIDES += \
	persist.sys.timezone=America/New_York \
	persist.sys.language=en \
	persist.sys.country=US

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=160 \
	persist.sys.usb.config=mass_storage,adb \
	ro.opengles.version=131072 \
	dalvik.vm.heapgrowthlimit=96m \
	ro.kernel.android.checkjni=0 \
	ro.udisk.lable=IBEN-L1 \
    	ro.product.firmware=v2.0 \
    	ro.sf.hwrotation=270

$(call inherit-product-if-exists, device/softwinner/fiber-hklt/modules/modules.mk)

DEVICE_PACKAGE_OVERLAYS := device/softwinner/fiber-hklt/overlay
PRODUCT_CHARACTERISTICS := tablet

# Overrides
CUSTOM_LOCALES := xhdpi mdpi
PRODUCT_BRAND  := IBEN-L1
PRODUCT_NAME   := fiber_hklt
PRODUCT_DEVICE := fiber-hklt
PRODUCT_MODEL  := IBEN-L1

include device/softwinner/fiber-common/prebuild/google/products/gms.mk


