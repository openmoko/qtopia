CONFIG		+= qtopiaplugin

TARGET		= lan

HEADERS		= lan.h
!buildSingleexec:HEADERS += ../proxiespage.h
SOURCES		= lan.cpp
!buildSingleexec:SOURCES += ../proxiespage.cpp
INTERFACES	= lanbase.ui lanstate.ui
!buildSingleexec:INTERFACES += ../proxiespagebase_p.ui

# Supported, but not enabled (tested hardward doesn't have Ethernet)
!QTOPIA_PHONE {
conf.files	= $${QTOPIA_DEPOT_PATH}/etc/network/LAN.conf
conf.path	= /etc/network
INSTALLS += conf
}

pics.files	= $${QTOPIA_DEPOT_PATH}/pics/Network/lan
pics.path	= /pics/Network

PICS_INSTALLS	+= pics

TRANSLATABLES = lan.h \
                ../proxiespage.h \
                lan.cpp \
                lanbase.ui \
                lanstate.ui \
                ../proxiespage.cpp \
                ../proxiespagebase_p.ui

