!qbuild{
qtopia_project(qtopia app)
TARGET=mediaserver
CONFIG+=singleexec_main

depends(libraries/qtopiamedia)
depends(libraries/qtopiaaudio)
enable_telephony:depends(libraries/qtopiaphone)

DEFINES+="CONFIGURED_ENGINES=$$LITERAL_QUOTE$$LITERAL_ESCAPED_QUOTE$$QTOPIAMEDIA_ENGINES$$LITERAL_ESCAPED_QUOTE$$LITERAL_QUOTE"
}

# Give us a direct connection to the document system
DEFINES+=QTOPIA_DIRECT_DOCUMENT_SYSTEM_CONNECTION

HEADERS =   \
            sessionmanager.h \
            engineloader.h \
            buildermanager.h \
            buildernegotiator.h \
            urinegotiator.h \
            mediaagent.h \
            qsoundprovider.h \
            qtopiamediaprovider.h \
            mediacontrolserver.h \
            mediacontentserver.h \
            domainmanager.h \
            sessionmanagersession.h \
            mediaagentsession.h \
            drmsession.h \
            mediapowercontrol.h\
            mediavolumecontrol.h \
            qaudiointerfaceserver.h

SOURCES =   \
            main.cpp \
            sessionmanager.cpp \
            engineloader.cpp \
            buildermanager.cpp \
            buildernegotiator.cpp \
            urinegotiator.cpp \
            mediaagent.cpp \
            qsoundprovider.cpp \
            qtopiamediaprovider.cpp \
            mediacontrolserver.cpp \
            mediacontentserver.cpp \
            domainmanager.cpp \
            sessionmanagersession.cpp \
            mediaagentsession.cpp \
            drmsession.cpp \
            mediapowercontrol.cpp\
            mediavolumecontrol.cpp \
            qaudiointerfaceserver.cpp
 
enable_telephony {
    HEADERS += callmonitor.h
    SOURCES += callmonitor.cpp
}

equals(QTOPIA_SOUND_SYSTEM,alsa) {
DEFINES+=QTOPIA_HAVE_ALSA
}
equals(QTOPIA_SOUND_SYSTEM,pulse) {
DEFINES+=QTOPIA_HAVE_ALSA
}


pkg.desc = Media Server
pkg.domain = trusted 

mediaserverservice.files = $$QTOPIA_DEPOT_PATH/services/MediaServer/mediaserver
mediaserverservice.path = /services/MediaServer
INSTALLS += mediaserverservice

domainmanager.files = $$device_overrides(/etc/default/Trolltech/AudioDomains.conf)
domainmanager.path = /etc/default/Trolltech
INSTALLS += domainmanager
