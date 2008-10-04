!qbuild{
qtopia_project(qtopia plugin)
TARGET=helix
CONFIG+=no_tr
}

CONFIG-=warn_on
CONFIG+=warn_off

HEADERS	= \
        helixenginefactory.h \
        helixengine.h \
        helixplayer.h \
        helixsession.h \
        helixsite.h \
        helixutil.h \
        helixvideosurface.h \
        helixvideowidget.h \
        qmediahelixsettingsserver.h \
        reporterror.h \
        interfaces.h \
        observer.h \
        config.h

SOURCES	= \
        helixenginefactory.cpp \
        helixengine.cpp \
        helixplayer.cpp \
        helixsession.cpp \
        helixsite.cpp \
        helixutil.cpp \
        helixvideosurface.cpp \
        helixvideowidget.cpp \
        qmediahelixsettingsserver.cpp \
        reporterror.cpp \
        iids.cpp \


helix_settings.files=helix.conf
helix_settings.path=/etc/default/Trolltech
INSTALLS += helix_settings

!qbuild{
    HELIX_PATH=$$fromfile($$QTOPIA_DEPOT_PATH/src/3rdparty/libraries/helix/helix.pro,HELIX_PATH)
    VPATH += $$HELIX_PATH/common/util \
             $$HELIX_PATH/video/vidutil
    SOURCES += HXErrorCodeStrings.c \
                colormap.c

dbg=$$HELIX_OUT_DIR
idep(DEFINES+=CONFIG_H_FILE=\$$LITERAL_ESCAPED_QUOTE$$HELIX_PATH/$$dbg/makefile_ribodefs.h\$$LITERAL_ESCAPED_QUOTE,DEFINES)
depends(3rdparty/libraries/helix)
depends(libraries/qtopiamedia)
depends(libraries/qtopiavideo)
}

