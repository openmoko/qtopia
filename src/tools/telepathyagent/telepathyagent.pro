!qbuild{
qtopia_project(qtopia app)
TARGET=telepathyagent
CONFIG+=no_tr no_singleexec
!no_singleexec:CONFIG+=singleexec_main
depends(libraries/qtopiaphone)
requires(enable_dbus)
depends(3rdparty/libraries/qtdbus)
depends(libraries/qtopiacollective)
depends(libraries/qtopiapim)
}

HEADERS		= \
    service.h \
    telepathychannel.h \
    telepathychannelinterfacegroup.h \
    telepathychanneltypetext.h \
    telepathyconnection.h \
    telepathyconnectioninterfacealiasing.h \
    telepathyconnectioninterfaceavatars.h \
    telepathyconnectioninterfacecapabilities.h \
    telepathyconnectioninterfacepresence.h \
    telepathyconnectionmanager.h \
    telepathynamespace.h

SOURCES		= \
    main.cpp \
    service.cpp \
    telepathychannel.cpp \
    telepathychannelinterfacegroup.cpp \
    telepathychanneltypetext.cpp \
    telepathyconnection.cpp \
    telepathyconnectioninterfacealiasing.cpp \
    telepathyconnectioninterfaceavatars.cpp \
    telepathyconnectioninterfacecapabilities.cpp \
    telepathyconnectioninterfacepresence.cpp \
    telepathyconnectionmanager.cpp \
    telepathynamespace.cpp

pkg.desc=Telepathy agent
pkg.domain=trusted

telepathyservice.files=$$QTOPIA_DEPOT_PATH/services/Telephony/telepathyagent
telepathyservice.path=/services/Telephony
INSTALLS+=telepathyservice
