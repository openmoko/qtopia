CONFIG		+= qtopiaplugin

TARGET		= dialup

HEADERS		= dialup.h
!buildSingleexec:HEADERS += ../proxiespage.h
SOURCES		= dialup.cpp
!buildSingleexec:SOURCES += ../proxiespage.cpp

QTOPIA_PHONE{
    INTERFACES  = dialupbase_phone.ui
} else {
    INTERFACES	= dialupbase.ui
}

!buildSingleexec:INTERFACES += ../proxiespagebase_p.ui

bin.files	= $${QTOPIA_DEPOT_PATH}/bin/qtopia-dial-internal \
		  $${QTOPIA_DEPOT_PATH}/bin/qtopia-stop-internal
bin.path	= /bin
!QTOPIA_PHONE:conf.files = \
	$${QTOPIA_DEPOT_PATH}/etc/network/DialUpIR.conf \
	$${QTOPIA_DEPOT_PATH}/etc/network/DialUp.conf
QTOPIA_PHONE:conf.files = \
	$${QTOPIA_DEPOT_PATH}/etc/network/DialUpGPRS*.conf \
	$${QTOPIA_DEPOT_PATH}/etc/network/DialUpGSM*.conf
conf.path	= /etc/network
pics.files	= $${QTOPIA_DEPOT_PATH}/pics/Network/dialup
pics.path	= /pics/Network

INSTALLS	+= bin conf
PICS_INSTALLS+=pics

TRANSLATABLES = dialup.h \
                ../proxiespage.h \
                dialup.cpp \
                ../proxiespage.cpp \
                dialupbase.ui \
                ../proxiespagebase_p.ui \
                dialupbase_phone.ui

