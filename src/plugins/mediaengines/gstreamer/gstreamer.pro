!qbuild{
qtopia_project(qtopia plugin)
TARGET = gstreamer
CONFIG += no_tr
LIBS += -lgstvideo-0.10 -lqtopiamedia
depends(libraries/qtopiamedia)
depends(libraries/qtopiavideo)
depends(3rdparty/libraries/gstreamer)
}

HEADERS	= \
        gstreamerengine.h \
        gstreamerengineinformation.h \
        gstreamerenginefactory.h \
        gstreamerurisessionbuilder.h \
        gstreamerplaybinsession.h \
        gstreamervideowidget.h \
        gstreamerqtopiavideosink.h \
        gstreamerbushelper.h \
        gstreamermessage.h \
        gstreamersinkwidget.h \
        gstreamerrtpsession.h \
        gstreamerqtopiacamerasource.h

SOURCES	= \
        gstreamerengine.cpp \
        gstreamerengineinformation.cpp \
        gstreamerenginefactory.cpp \
        gstreamerurisessionbuilder.cpp \
        gstreamerplaybinsession.cpp \
        gstreamervideowidget.cpp \
        gstreamerqtopiavideosink.cpp \
        gstreamerbushelper.cpp \
        gstreamermessage.cpp \
        gstreamersinkwidget.cpp \
        gstreamerrtpsession.cpp \
        gstreamerqtopiacamerasource.cpp

gstreamer_settings.files=$$device_overrides($$project()gstreamer.conf)
gstreamer_settings.path=/etc/default/Trolltech
INSTALLS += gstreamer_settings
