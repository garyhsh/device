$(call inherit-product, device/softwinner/fiber-common/fiber-common.mk)
$(call inherit-product, frameworks/native/build/tablet-7in-hdpi-1024-dalvik-heap.mk)

# init.rc, kernel
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-w02/kernel:kernel \
	device/softwinner/fiber-w02/modules/modules/nand.ko:root/nand.ko \
	device/softwinner/fiber-w02/init.sun6i.rc:root/init.sun6i.rc \
	device/softwinner/fiber-w02/ueventd.sun6i.rc:root/ueventd.sun6i.rc \
	device/softwinner/fiber-w02/initlogo.rle:root/initlogo.rle  \
	device/softwinner/fiber-w02/needfix.rle:root/needfix.rle  \
	device/softwinner/fiber-w02/media/bootanimation.zip:system/media/bootanimation.zip \
	device/softwinner/fiber-w02/media/boot.wav:system/media/boot.wav

#key and tp config file
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-w02/sw-keyboard.kl:system/usr/keylayout/sw-keyboard.kl \
	device/softwinner/fiber-w02/gslX680.idc:system/usr/idc/gslX680.idc \
	device/softwinner/fiber-w02/gsensor.cfg:system/usr/gsensor.cfg

#copy touch and keyboard driver to recovery randisk
PRODUCT_COPY_FILES += \
  device/softwinner/fiber-w02/modules/modules/gslX680.ko:obj/touch.ko \
  device/softwinner/fiber-w02/modules/modules/sw-keyboard.ko:obj/keyboard.ko \
  device/softwinner/fiber-w02/modules/modules/disp.ko:obj/disp.ko \
  device/softwinner/fiber-w02/modules/modules/lcd.ko:obj/lcd.ko \
  device/softwinner/fiber-w02/modules/modules/hdmi.ko:obj/hdmi.ko

# wifi & bt config file
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml

#vold config
PRODUCT_COPY_FILES += \
   device/softwinner/fiber-w02/vold.fstab:system/etc/vold.fstab \
   device/softwinner/fiber-w02/recovery.fstab:recovery.fstab

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
	device/softwinner/fiber-w02/camera.cfg:system/etc/camera.cfg \
	device/softwinner/fiber-w02/media_profiles.xml:system/etc/media_profiles.xml \
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

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=320 \
	persist.sys.usb.config=mass_storage,adb \
	ro.opengles.version=131072 \
	dalvik.vm.heapgrowthlimit=96m \
	ro.udisk.lable=FIBER \
	ro.kernel.android.checkjni=0 \
    ro.product.firmware=v2.0



$(call inherit-product-if-exists, device/softwinner/fiber-w02/modules/modules.mk)

DEVICE_PACKAGE_OVERLAYS := device/softwinner/fiber-w02/overlay
PRODUCT_CHARACTERISTICS := tablet

# Overrides
#CUSTOM_LOCALES := xhdpi mdpi
PRODUCT_AAPT_CONFIG := normal mdpi xhdpi
PRODUCT_AAPT_PREF_CONFIG := xhdpi
PRODUCT_BRAND  := Softwinner
PRODUCT_NAME   := fiber_w02
PRODUCT_DEVICE := fiber-w02
PRODUCT_MODEL  := Softwinner

include device/softwinner/fiber-common/prebuild/google/products/gms.mk

