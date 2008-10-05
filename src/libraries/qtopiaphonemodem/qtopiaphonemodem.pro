!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiaphonemodem
CONFIG += qtopia_visibility no_tr
depends(libraries/qtopiaphone)
}

HEADERS= \
    qmodemservice.h \
    qmodemserviceplugin.h \
    qmodemnetworkregistration.h \
    qmodempreferrednetworkoperators.h \
    qmodemcallbarring.h \
    qmodemcallforwarding.h \
    qmodemcallsettings.h \
    qmodemsmsreader.h \
    qmodemsmssender.h \
    qmodempinmanager.h \
    qmodemcall.h \
    qmodemcallprovider.h \
    qmodemsimtoolkit.h \
    qmodemphonebook.h \
    qmodemcellbroadcast.h \
    qmodemservicenumbers.h \
    qmodemsupplementaryservices.h \
    qmodemsiminfo.h \
    qmodemindicators.h \
    qmodemsimfiles.h \
    qmodemsimgenericaccess.h \
    qmodemadviceofcharge.h \
    qmodemconfiguration.h \
    qmodemvibrateaccessory.h \
    qmodemrffunctionality.h \
    qmodemgprsnetworkregistration.h \
    qmodemcallvolume.h
PRIVATE_HEADERS=\
    qmodempinmanager_p.h \
    qmodemsimfiles_p.h \
    qmodempppdmanager_p.h
SOURCES=\
    qmodemservice.cpp \
    qmodemserviceplugin.cpp \
    qmodemnetworkregistration.cpp \
    qmodempreferrednetworkoperators.cpp \
    qmodemcallbarring.cpp \
    qmodemcallforwarding.cpp \
    qmodemcallsettings.cpp \
    qmodemsmsreader.cpp \
    qmodemsmssender.cpp \
    qmodempinmanager.cpp \
    qmodemcall.cpp \
    qmodemcallprovider.cpp \
    qmodemsimtoolkit.cpp \
    qmodemphonebook.cpp \
    qmodemcellbroadcast.cpp \
    qmodemservicenumbers.cpp \
    qmodemsupplementaryservices.cpp \
    qmodemsiminfo.cpp \
    qmodemindicators.cpp \
    qmodemsimfiles.cpp \
    qmodemsimgenericaccess.cpp \
    qmodemadviceofcharge.cpp \
    qmodemconfiguration.cpp \
    qmodemvibrateaccessory.cpp \
    qmodemrffunctionality.cpp \
    qmodemgprsnetworkregistration.cpp \
    qmodempppdmanager.cpp \
    qmodemcallvolume.cpp

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopiaphonemodem
headers.hint=sdk headers
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
