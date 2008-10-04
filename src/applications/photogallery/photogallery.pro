!qbuild {
qtopia_project(qtopia app)
TARGET=photogallery
enable_pictureflow:depends(3rdparty/libraries/pictureflow)
depends(libraries/homeui)
depends(libraries/qtopiagfx)
}

enable_pictureflow:MODULES*=pictureflow

# Input
HEADERS += albummodel.h \
           albumribbon.h \
           albumselector.h \
           durationslider.h \
           imageselector.h \
           imageview.h \
           imageloader.h \
           photogallery.h \
           qsmoothiconview.h \
           slideshowview.h \
           smoothimagemover.h \
           titlewindow.h \
           thumbcache.h \
           thumbmodel.h \
           zoomslider.h


SOURCES += albummodel.cpp \
           albumribbon.cpp \
           albumselector.cpp \
           durationslider.cpp \
           imageselector.cpp \
           imageview.cpp \
           imageloader.cpp \
           photogallery.cpp \
           qsmoothiconview.cpp \
           slideshowview.cpp \
           smoothimagemover.cpp \
           titlewindow.cpp \
           thumbcache.cpp \
           thumbmodel.cpp \
           zoomslider.cpp \
           main.cpp

desktop.files=photogallery.desktop
desktop.path=/apps/Applications
desktop.hint=nct desktop
desktop.trtarget=photogallery-nct
INSTALLS+=desktop

service.files=$$QTOPIA_DEPOT_PATH/services/PhotoGallery/photogallery
service.path=/services/PhotoGallery
INSTALLS+=service

pics.files=$$QTOPIA_DEPOT_PATH/pics/photoedit/*
pics.path=/pics/photogallery
pics.hint=pics
INSTALLS+=pics

# Set this to trusted for full privileges
target.hint=sxe
target.domain=trusted

