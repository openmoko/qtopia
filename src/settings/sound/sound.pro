CONFIG		+= qtopiaapp

HEADERS		= soundsettings.h
SOURCES		= soundsettings.cpp main.cpp
INTERFACES	= soundsettingsbase.ui
TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= sound

desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Sound.desktop
desktop.path=/apps/Settings
pics.files=$${QTOPIA_DEPOT_PATH}/pics/sound/*
pics.path=/pics/sound/
INSTALLS	+= desktop
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Sound settings dialog for the Qtopia environment.
