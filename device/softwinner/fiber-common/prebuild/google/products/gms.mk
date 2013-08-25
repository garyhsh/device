# Add Google Apps and Services
PRODUCT_PACKAGES += \
     Books \
     ChromeWithBrowser \
     ChromeBookmarksSyncAdapter \
     FaceLock \
     GenieWidget \
     Gmail \
     GoogleBackupTransport \
     GoogleCalendarSyncAdapter \
     GoogleContactsSyncAdapter \
     GoogleFeedback \
     GmsCore \
     GoogleLoginService \
     GooglePartnerSetup \
     GoogleServicesFramework \
     LatinImeDictionaryPack \
     Magazines \
     GMS_Maps \
     MediaUploader \
     Music2 \
     NetworkLocation \
     OneTimeInitializer \
     Phonesky \
     PlusOne \
     SetupWizard \
     Street \
     Talk \
     talkback \
     Videos \
     VoiceSearchStub \
     YouTube \
     com.google.android.media.effects.jar \
     google_generic_update.txt

# Add in the Google version of the platform apps
PRODUCT_PACKAGES += \
     CalendarGoogle \
     DeskClockGoogle \
     GalleryGoogle \
     LatinImeGoogle \
     TagGoogle \
     Velvet \
     VideoEditorGoogle
     

# Include GoogleTTS and TTS languages needed for GoogleTTS
PRODUCT_PACKAGES += GoogleTTS
include device/softwinner/fiber-common/prebuild/google/products/text_to_speech_languages.mk 

# Overrides
PRODUCT_PROPERTY_OVERRIDES += \
     ro.setupwizard.mode=OPTIONAL \
     ro.com.google.gmsversion=4.1_r4
