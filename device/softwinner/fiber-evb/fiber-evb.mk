$(call inherit-product, device/softwinner/fiber-common/fiber-common.mk)
$(call inherit-product, frameworks/native/build/tablet-dalvik-heap.mk)

# init.rc, kernel
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-evb/kernel:kernel \
	device/softwinner/fiber-evb/init.sun6i.rc:root/init.sun6i.rc \
	device/softwinner/fiber-evb/ueventd.sun6i.rc:root/ueventd.sun6i.rc \
	device/softwinner/fiber-evb/modules/modules/nand.ko:root/nand.ko \
	device/softwinner/fiber-evb/initlogo.rle:root/initlogo.rle \
	device/softwinner/fiber-evb/needfix.rle:root/needfix.rle \
	device/softwinner/fiber-evb/media/bootanimation.zip:system/media/bootanimation.zip \
	device/softwinner/fiber-evb/media/boot.wav:system/media/boot.wav


#key and tp config file
PRODUCT_COPY_FILES += \
	device/softwinner/fiber-evb/sw-keyboard.kl:system/usr/keylayout/sw-keyboard.kl \
	device/softwinner/fiber-evb/gt82x.idc:system/usr/idc/gt82x.idc \
	device/softwinner/fiber-evb/gsensor.cfg:system/usr/gsensor.cfg

#copy touch and keyboard driver to recovery randisk
PRODUCT_COPY_FILES += \
  device/softwinner/fiber-evb/modules/modules/gt82x.ko:obj/touch.ko \
  device/softwinner/fiber-evb/modules/modules/sw-keyboard.ko:obj/keyboard.ko\
  device/softwinner/fiber-evb/modules/modules/disp.ko:obj/disp.ko\
  device/softwinner/fiber-evb/modules/modules/lcd.ko:obj/lcd.ko\
  device/softwinner/fiber-evb/modules/modules/hdmi.ko:obj/hdmi.ko

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

# rtl8723au bt fw and config
#PRODUCT_COPY_FILES += \
#	device/softwinner/fiber-common/hardware/realtek/bluetooth/rtl8723au/rtk8723a:system/etc/firmware/rtk8723a \
#	device/softwinner/fiber-common/hardware/realtek/bluetooth/rtl8723au/rtk8723_bt_config:system/etc/firmware/rtk8723_bt_config

# ap6181 sdio wifi fw and nvram
#PRODUCT_COPY_FILES += \
#	hardware/broadcom/wlan/firmware/ap6181/fw_bcm40181a2_p2p.bin:system/vendor/modules/fw_bcm40181a2_p2p.bin \
#	hardware/broadcom/wlan/firmware/ap6181/fw_bcm40181a2_apsta.bin:system/vendor/modules/fw_bcm40181a2_apsta.bin \
#	hardware/broadcom/wlan/firmware/ap6181/fw_bcm40181a2.bin:system/vendor/modules/fw_bcm40181a2.bin \
#	hardware/broadcom/wlan/firmware/ap6181/nvram_ap6181.txt:system/vendor/modules/nvram_ap6181.txt

# ap6210 sdio wifi fw and nvram
#PRODUCT_COPY_FILES += \
#	hardware/broadcom/wlan/firmware/ap6210/fw_bcm40181a2.bin:system/vendor/modules/fw_bcm40181a2.bin \
#	hardware/broadcom/wlan/firmware/ap6210/fw_bcm40181a2_apsta.bin:system/vendor/modules/fw_bcm40181a2_apsta.bin \
#	hardware/broadcom/wlan/firmware/ap6210/fw_bcm40181a2_p2p.bin:system/vendor/modules/fw_bcm40181a2_p2p.bin \
#	hardware/broadcom/wlan/firmware/ap6210/nvram_ap6210.txt:system/vendor/modules/nvram_ap6210.txt \
#	hardware/broadcom/wlan/firmware/ap6210/bcm20710a1.hcd:system/vendor/modules/bcm20710a1.hcd

#vold config
PRODUCT_COPY_FILES += \
   device/softwinner/fiber-evb/vold.fstab:system/etc/vold.fstab \
   device/softwinner/fiber-evb/recovery.fstab:recovery.fstab

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
	device/softwinner/fiber-evb/camera.cfg:system/etc/camera.cfg \
	device/softwinner/fiber-evb/media_profiles.xml:system/etc/media_profiles.xml \
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
	ro.product.rtl8723au=no-use \
	ro.sf.lcd_density=160 \
	persist.sys.usb.config=mass_storage,adb \
	ro.opengles.version=131072 \
	ro.udisk.lable=fiber \
    ro.product.firmware=v2.0


$(call inherit-product-if-exists, device/softwinner/fiber-evb/modules/modules.mk)

DEVICE_PACKAGE_OVERLAYS := device/softwinner/fiber-evb/overlay
PRODUCT_CHARACTERISTICS := tablet

# Overrides
PRODUCT_BRAND  := softwinners
PRODUCT_NAME   := fiber_evb
PRODUCT_DEVICE := fiber-evb
PRODUCT_MODEL  := Softwiner

include device/softwinner/fiber-common/prebuild/google/products/gms.mk


