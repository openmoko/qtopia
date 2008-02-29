TEMPLATE = app
TARGET = voipuseragent

DEFINES += SHARE_DIR="\"./\""
INCLUDEPATH += ../dissipate2 ../gsm ../ilbc ../../../../libraries/qtopiaphone \
	       ../../../../applications/voip

LIBS += -Wl,-whole-archive
LIBS += -ldissipate2 -lssl -lcrypto -lkgsm -lilbc -lqtopia2 -lqtopia -lqpe -luuid -lcrypt
LIBS += -Wl,-no-whole-archive

CONFIG-=buildQuicklaunch
target.path=/bin
INSTALLS+=target

SOURCES += audiobuffer.cpp \
audiocontrol.cpp \
callaudio.cpp \
calllistview.cpp \
dspaudio.cpp \
dspaudioin.cpp \
dspaudioout.cpp \
dspout.cpp \
dspoutoss.cpp \
dspoutrtp.cpp \
dtmfgenerator.cpp \
g711.cpp \
kcallwidget.cpp \
kphonebook.cpp \
kphone.cpp \
kphoneview.cpp \
ksipauthentication.cpp \
ksipoptions.cpp \
ksippreferences.cpp \
ksipregistrations.cpp \
kmain.cpp \
menulistview.cpp \
qsettings.cpp \
voipuseragent.cpp \
main.cpp \
voipuseragentstructures.cpp\
voipuseragentapp.cpp \
../../../../libraries/qtopiaphone/phonesocket.cpp \
../../../../applications/voip/voipfw_settings.cpp

HEADERS += audiobuffer.h \
audiocontrol.h \
callaudio.h \
calllistview.h \
codec.h \
dspaudio.h \
dspaudioin.h \
dspaudioout.h \
dspout.h \
dspoutoss.h \
dspoutrtp.h \
dtmfgenerator.h \
g711.h \
kcallwidget.h \
kphonebook.h \
kphone.h \
kphoneview.h \
ksipauthentication.h \
ksipoptions.h \
ksippreferences.h \
ksipregistrations.h \
menulistview.h \
rtpdataheader.h \
qsettings.h \
voipuseragent.h\
voipuseragentapp.h \
../../../../libraries/qtopiaphone/phonesocket.h \
../../../../applications/voip/voipfw_settings.cpp

TRANSLATABLES = $${SOURCES} $${HEADERS}

phonehandler.files=$${QTOPIA_DEPOT_PATH}/services/VoIP/voipuseragent
phonehandler.path=/services/VoIP

INSTALLS+=phonehandler
