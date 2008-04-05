qtopia_project(qtopia plugin)

#
TARGET = gstreamer
CONFIG += no_tr

HEADERS	= \
        gstreamerengine.h \
        gstreamerengineinformation.h \
        gstreamerenginefactory.h \
        gstreamerurisessionbuilder.h \
        gstreamerplaybinsession.h \
        gstreamerqtopiavideosink.h \
        gstreamerbushelper.h \
        gstreamermessage.h \
        gstreamersinkwidget.h

SOURCES	= \
        gstreamerengine.cpp \
        gstreamerengineinformation.cpp \
        gstreamerenginefactory.cpp \
        gstreamerurisessionbuilder.cpp \
        gstreamerplaybinsession.cpp \
        gstreamerqtopiavideosink.cpp \
        gstreamerbushelper.cpp \
        gstreamermessage.cpp \
        gstreamersinkwidget.cpp

x11 {
    HEADERS += gstreamerx11painterwidget.h
    SOURCES += gstreamerx11painterwidget.cpp
} else {
    HEADERS += gstreamerdirectpainterwidget.h
    SOURCES += gstreamerdirectpainterwidget.cpp
}

LIBS += -lgstvideo-0.10
depends(libraries/qtopiamedia)
depends(3rdparty/libraries/gstreamer)

#
gstreamer_settings.files=$$QTOPIA_DEPOT_PATH/src/plugins/mediaengines/gstreamer/gstreamer.conf
gstreamer_settings.path=/etc/default/Trolltech
INSTALLS += gstreamer_settings
