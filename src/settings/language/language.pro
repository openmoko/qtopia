CONFIG		+= qtopiaapp

HEADERS		= languagesettings.h
SOURCES		= language.cpp main.cpp
INTERFACES	= languagesettingsbase.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= language

help.files=$${QTOPIA_DEPOT_PATH}/help/html/language.html
help.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Language.desktop
desktop.path=/apps/Settings
pics.files=$${QTOPIA_DEPOT_PATH}/pics/language/*
pics.path=/pics/language
INSTALLS	+= desktop help
PICS_INSTALLS+=pics

