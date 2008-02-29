qtopia_project(qtopia lib)
TARGET=qtopiamedia
CONFIG+=no_pkg qtopia_visibility no_tr

HEADERS += media.h

PRIVATE_HEADERS += mediastyle_p.h \
            servicerequest_p.h \
            requesthandler_p.h \
            menumodel_p.h \
            menuview_p.h \
            browser_p.h \
            activitymonitor_p.h \
            keyhold_p.h \
            observer_p.h \
            qmediahandle_p.h \
            qmediacontentplayer_p.h

HEADERS += qmediatools.h \
            qmediawidgets.h \

HEADERS += \
            qmediaabstractcontrol.h \
            qmediacontrol.h \
            qmediavideocontrol.h \
            qmediaseekcontrol.h \
            qmediacontent.h \
            qmediapipe.h \
            qmediadevice.h \
            qmediaencoder.h \
            qmediadecoder.h \
            qmediacodecplugin.h

HEADERS +=  qaudiodomain.h \
            qdomainmanagerconfiguration.h

HEADERS +=  private/mediaserverproxy_p.h

HEADERS += \
            server\qmediaenginefactory.h \
            server\qmediaengine.h \
            server\qmediaengineinformation.h \
            server\qmediaserversession.h \
            server\qmediasessionbuilder.h \
            server\qmediasessionrequest.h \
            server\qmediaabstractcontrolserver.h \
            server\qmediavideocontrolserver.h \
            server\qmediaseekcontrolserver.h

            
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

SOURCES += \
            qmediaabstractcontrol.cpp \
            qmediacontrol.cpp \
            qmediavideocontrol.cpp \
            qmediaseekcontrol.cpp \
            qmediacontent.cpp \
            qmediacontentplayer.cpp \
            qmediapipe.cpp \
            qmediacodecplugin.cpp \
            qmediadecoder.cpp \
            qmediaencoder.cpp

SOURCES +=  qaudiodomain.cpp \
            qdomainmanagerconfiguration.cpp

SOURCES += private/mediaserverproxy.cpp

SOURCES += \
            server\qmediaengine.cpp \
            server\qmediaengineinformation.cpp \
            server\qmediaserversession.cpp \
            server\qmediasessionbuilder.cpp \
            server\qmediasessionrequest.cpp \
            server\qmediaabstractcontrolserver.cpp \
            server\qmediavideocontrolserver.cpp \
            server\qmediaseekcontrolserver.cpp

contains(QTOPIAMEDIA_ENGINES,helix) {
    HEADERS += qmediahelixsettingscontrol.h
    SOURCES += qmediahelixsettingscontrol.cpp
}

depends(libraries/qtopia)

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)

PREFIX=QTOPIAMEDIA
resolve_include()

headers.files = $$HEADERS
headers.path = /include/qtopia/qtopiamedia
headers.hint = headers sdk

private_headers.files = $$PRIVATE_HEADERS
private_headers.path = /include/qtopia/qtopiamedia/private
private_headers.hint = headers sdk

INSTALLS += headers private_headers
