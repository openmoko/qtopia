CONFIG		+= qtopiaapp
TARGET		= voip

# For some reason, this can't be build quicklaunched...
CONFIG-=buildQuicklaunch

HEADERS		= voipmain.h voipidentity.h voipsettings.h \
		  voipconfig.h voipuseragentstructures.h \
		  voipfw_settings.h
SOURCES		= main.cpp voipmain.cpp voipidentity.cpp voipsettings.cpp \
		  voipconfig.cpp voipuseragentstructures.cpp \
		  voipfw_settings.cpp

desktop.files = $${QTOPIA_DEPOT_PATH}/apps/Applications/voip.desktop
desktop.path = /apps/Applications
INSTALLS += desktop

pics.files=$${QTOPIA_DEPOT_PATH}/pics/voip/*
pics.path=/pics/voip
PICS_INSTALLS+=pics

help.files=$${QTOPIA_DEPOT_PATH}/help/html/voip*
help.path=/help/html
INSTALLS+=help

TRANSLATABLES = $$HEADERS $$SOURCES $$INTERFACES

