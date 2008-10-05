!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiamedia
CONFIG+=qtopia_visibility no_tr
depends(libraries/qtopiavideo)
}


HEADERS += media.h

PRIVATE_HEADERS += mediastyle_p.h \
            qmediacontentplayer_p.h

SEMI_PRIVATE_HEADERS+=\
    qmediahandle_p.h\
    activitymonitor_p.h\
    observer_p.h \
    qmediametainfocontrol_p.h

HEADERS += qmediatools.h \
            qmediawidgets.h

HEADERS += \
            qmediaabstractcontrol.h \
            qmediacontrol.h \
            qmediavideocontrol.h \
            qmediaseekcontrol.h \
            qmediacontent.h \
            qmediadevice.h \
            qmediaencoder.h \
            qmediadecoder.h \
            qmediacodecplugin.h\
            qmediartpsessionengine.h \
            qmediartpsession.h \
            qmarqueelabel.h \
            qmediaplaylist.h \
            qmedialist.h \
            qmediamenu.h


PRIVATE_HEADERS +=  private/mediaserverproxy_p.h

HEADERS += \
            server/qmediaenginefactory.h \
            server/qmediaengine.h \
            server/qmediaengineinformation.h \
            server/qmediaserversession.h \
            server/qmediasessionbuilder.h \
            server/qmediasessionrequest.h \
            server/qmediaabstractcontrolserver.h \
            server/qmediavideocontrolserver.h \
            server/qmediaseekcontrolserver.h

SOURCES = media.cpp

SOURCES += mediastyle.cpp \
            qmediatools.cpp \
            qmediawidgets.cpp \
            activitymonitor.cpp \
            qmediametainfocontrol.cpp

SOURCES += \
            qmediaabstractcontrol.cpp \
            qmediacontrol.cpp \
            qmediavideocontrol.cpp \
            qmediaseekcontrol.cpp \
            qmediacontent.cpp \
            qmediacontentplayer.cpp \
            qmediacodecplugin.cpp \
            qmediadecoder.cpp \
            qmediaencoder.cpp \
            qmediartpsessionengine.cpp \
            qmediartpsession.cpp \
            qmarqueelabel.cpp \
            qmediaplaylist.cpp \
            qmedialist.cpp \
            qmediamenu.cpp

SOURCES += \
            qmediahandle.cpp \
            private/mediaserverproxy.cpp

SOURCES += \
            server/qmediaengine.cpp \
            server/qmediaengineinformation.cpp \
            server/qmediaserversession.cpp \
            server/qmediasessionbuilder.cpp \
            server/qmediasessionrequest.cpp \
            server/qmediaabstractcontrolserver.cpp \
            server/qmediavideocontrolserver.cpp \
            server/qmediaseekcontrolserver.cpp

HELIX.TYPE=CONDITIONAL_SOURCES
HELIX.CONDITION=contains(QTOPIAMEDIA_ENGINES,helix)
HELIX.HEADERS=qmediahelixsettingscontrol.h
HELIX.SOURCES=qmediahelixsettingscontrol.cpp
!qbuild:CONDITIONAL_SOURCES(HELIX)

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/qtopiamedia
headers.hint=headers sdk
INSTALLS+=headers

pheaders.files=$$SEMI_PRIVATE_HEADERS
pheaders.path=/include/qtopia/qtopiamedia/private
pheaders.hint=headers sdk
INSTALLS+=pheaders

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
