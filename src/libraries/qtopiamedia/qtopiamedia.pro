qtopia_project(qtopia lib)
TARGET=qtopiamedia
CONFIG+=no_pkg qtopia_visibility no_tr

HEADERS += media.h

HEADERS += mediastyle.h \
            qmediatools.h \
            qmediawidgets.h \
            servicerequest.h \
            requesthandler.h \
            menumodel.h \
            menuview.h \
            browser.h \
            activitymonitor.h \
            keyhold.h \
            observer.h

HEADERS += qmediasession.h \
            qmediacontrol.h \
            qmediavideocontrol.h \
            qmediahelixsettingscontrol.h \
            qmediahandle.h \
            qmediacontent.h \
            qmediacontentplayer.h \
            qmediacodecinfo.h \
            qmediacodecrep.h \
            qmediadeviceinfo.h \
            qmediadevicerep.h \
            qmediapipe.h \
            qmediadevice.h \
            qmediaencoder.h \
            qmediadecoder.h \
            qmediacodecplugin.h \
            qmediadevicemanager.h \
            qmediadevicemanagerfactory.h

HEADERS += private/mediaserverproxy.h
            
SOURCES = media.cpp

SOURCES += mediastyle.cpp \
            qmediatools.cpp \
            qmediawidgets.cpp \
            requesthandler.cpp \
            menumodel.cpp \
            menuview.cpp \
            browser.cpp \
            activitymonitor.cpp \
            keyhold.cpp

SOURCES += qmediasession.cpp \
            qmediacontrol.cpp \
            qmediavideocontrol.cpp \
            qmediahelixsettingscontrol.cpp \
            qmediacontent.cpp \
            qmediacontentplayer.cpp \
            qmediacodecinfo.cpp \
            qmediacodecrep.cpp \
            qmediadeviceinfo.cpp \
            qmediadevicerep.cpp \
            qmediapipe.cpp

SOURCES += private/mediaserverproxy.cpp

depends(libraries/qtopia)
!enable_qtopiabase:depends(libraries/qtopiail)

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)

headers.files = $$HEADERS
headers.path = /include/qtopia/qtopiamedia
headers.hint = headers

INSTALLS += headers
