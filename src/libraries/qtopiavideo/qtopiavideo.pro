!qbuild {
qtopia_project(qtopia lib)
TARGET=qtopiavideo
CONFIG+= qtopia_visibility no_tr
}

# Input
HEADERS = \
            qcamera.h \
            qcameracontrol.h \
            qcameradevice.h \
            qcameradeviceplugin.h \
            qcameradeviceloader.h \
            qcameratools.h

SOURCES = \
            qcameracontrol.cpp \
            qcameradeviceplugin.cpp \
            qcameradeviceloader.cpp \
            qcameratools.cpp

# Output
HEADERS += \
            qtopiavideo.h \
            qvideoframe.h \
            qimageplanetransform.h \
            qvideosurface.h

SOURCES += \
            qtopiavideo.cpp \
            qvideoframe.cpp \
            qimageplanetransform.cpp

!x11 {
    HEADERS += \
            qabstractvideooutput.h \
            qvideooutputloader.h \
            qvideooutputfactory.h

    SOURCES += \
            qabstractvideooutput.cpp \
            qvideooutputfactory.cpp \
            qvideooutputloader.cpp \
            qvideosurface.cpp

} else {
    HEADERS += \
            qgenericvideowidget.h

    SOURCES += \
            qvideosurface_x11.cpp \
            qgenericvideowidget.cpp
}


!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/qtopiavideo
headers.hint=headers sdk
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
