CONFIG		+= qtopiaapp

HEADERS		= languagesettings.h
SOURCES		= language.cpp main.cpp
INTERFACES	= languagesettingsbase.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= language

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=language.html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Language.desktop
desktop.path=/apps/Settings
pics.files=$${QTOPIA_DEPOT_PATH}/pics/language/*
pics.path=/pics/language
INSTALLS	+= desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

