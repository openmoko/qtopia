CONFIG		+= qtopiaapp

HEADERS		= ../../plugins/applets/irreceiver/ircontroller.h beaming.h
SOURCES		= ../../plugins/applets/irreceiver/ircontroller.cpp beaming.cpp main.cpp
INTERFACES	=

TARGET		= beaming

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=beaming*

pics.files=$${QTOPIA_DEPOT_PATH}/pics/beaming/*
pics.path=/pics/beaming
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Beaming.desktop
desktop.path=/apps/Settings
INSTALLS	+= desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Beaming settings dialog for the Qtopia environment.
