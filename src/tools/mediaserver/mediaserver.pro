qtopia_project(qtopia app)


TARGET=mediaserver
CONFIG+=no_tr

HEADERS =   \
            helixengine.h \
            helixsession.h \
            mediaengine.h \
            mediaserver.h \
            mediasession.h \
            sessionmanager.h \
            qsoundprovider.h \
            qtopiamediaprovider.h \
            mediacontrolserver.h \
            videocontrolserver.h \
            qmediahelixsettingsserver.h \
            drmsession.h \
            plugindecodesession.h \
            pluginencodesession.h \
            fullduplexpluginsession.h \
            devicemanager.h \
            contentdevice.h 


HEADERS +=  \
            helixutil.h \
            helixplayer.h \
            helixsite.h \
            helixvideosurface.h \
            reporterror.h

SOURCES =   \
            helixengine.cpp \
            helixsession.cpp \
            main.cpp \
            mediaengine.cpp \
            mediaserver.cpp \
            sessionmanager.cpp \
            qsoundprovider.cpp \
            qtopiamediaprovider.cpp \
            mediacontrolserver.cpp \
            videocontrolserver.cpp \
            qmediahelixsettingsserver.cpp \
            drmsession.cpp \
            plugindecodesession.cpp \
            pluginencodesession.cpp \
            fullduplexpluginsession.cpp \
            devicemanager.cpp \
            contentdevice.cpp

SOURCES +=  \
            helixutil.cpp \
            helixplayer.cpp \
            helixsite.cpp \
            helixvideosurface.cpp \
            reporterror.cpp \
            iids.cpp


# From helix
HELIX_PATH=$$fromfile($$QTOPIA_DEPOT_PATH/src/3rdparty/libraries/helix/helix.pro,HELIX_PATH)
VPATH += $$HELIX_PATH/common/util \
            $$HELIX_PATH/video/vidutil
SOURCES += HXErrorCodeStrings.c \
            colormap.c

dbg=$$HELIX_OUT_DIR
idep(DEFINES+=CONFIG_H_FILE=\$$LITERAL_ESCAPED_QUOTE$$HELIX_PATH/$$dbg/makefile_ribodefs.h\$$LITERAL_ESCAPED_QUOTE,DEFINES)

pkg.desc=Media Server
pkg.domain=mediaserver,window,graphics,docapi,launcher,drm,cardreader

mediaserverservice.files=$$QTOPIA_DEPOT_PATH/services/MediaServer/mediaserver
mediaserverservice.path=/services/MediaServer

INSTALLS += mediaserverservice

depends(libraries/qtopiamedia)
depends(libraries/qtopia)
!enable_qtopiabase:depends(libraries/qtopiail)
depends(3rdparty/libraries/helix)

