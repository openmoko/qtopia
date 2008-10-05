#This file contains projects that make up the Essentials module.

PROJECTS*=\
    settings/light-and-power \
    settings/security \
    applications/calculator \
    applications/clock \
    applications/sysinfo \
    applications/textedit \
    settings/language \
    settings/systemtime \
    settings/worldtime \
    plugins/content/exif \

!equals(QTOPIA_UI,home) {
    PROJECTS+=applications/photoedit
} else {
    PROJECTS+=applications/photogallery
}

!x11:PROJECTS*=\
    server/ui/components/calibrate \ #calibrate application
    server/infrastructure/keyboardlock   #qws based keyboard locking

PROJECTS*=\
    server/ui/components/firstuse \             #firstuse dialogs
    server/phone/powermanager \                 #power manager for suspend/lightoff/dim
    server/infrastructure/suspendtasks \        #default suspend handlers
    server/infrastructure/apm                   #apm impl for QPowerSource

# Camera
enable_pictureflow:PROJECTS*=3rdparty/libraries/pictureflow
PROJECTS*=\
    libraries/qtopiavideo \
    applications/camera \
    plugins/content/exif \                      #exif file support
    server/infrastructure/camera                #camera detection

enable_qvfb:PROJECTS*=plugins/cameras/v4lwebcams # webcam for qvfb builds
