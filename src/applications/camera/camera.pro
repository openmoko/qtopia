!qbuild {
qtopia_project(qtopia app)
DEFINES-=QT_NO_DEBUG 
TARGET=camera
CONFIG+=qtopia_main
depends(libraries/qtopiavideo)
enable_pictureflow:depends(3rdparty/libraries/pictureflow)
}

FORMS=camerabase.ui camerasettings.ui

HEADERS = mainwindow.h \
          phototimer.h \
          minsecspinbox.h\
          noeditspinbox.h\
          videocaptureview.h\
          camerastateprocessor.h\
          cameraformatconverter.h\
          zoomslider.h\
          cameravideosurface.h

SOURCES = mainwindow.cpp \
          main.cpp \
          phototimer.cpp \
          minsecspinbox.cpp\
          noeditspinbox.cpp\
          videocaptureview.cpp\
          camerastateprocessor.cpp\
          cameraformatconverter.cpp\
          zoomslider.cpp\
          cameravideosurface.cpp

enable_pictureflow {
    MODULES*=pictureflow
    HEADERS+=imagebrowser.h
    SOURCES+=imagebrowser.cpp
}



desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/camera.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=camera*
help.hint=help
INSTALLS+=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/camera/*
pics.path=/pics/camera
pics.hint=pics
INSTALLS+=pics
service.files=$$QTOPIA_DEPOT_PATH/services/Camera/camera
service.path=/services/Camera
INSTALLS+=service
qdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/Camera
qdsservice.path=/etc/qds
INSTALLS+=qdsservice

# the server does this for us
#categories.files=$$QTOPIA_DEPOT_PATH/etc/categories/camera.conf
#categories.trtarget=QtopiaCategories
#categories.hint=nct
#INSTALLS+=categories

pkg.desc=Camera
pkg.domain=trusted
