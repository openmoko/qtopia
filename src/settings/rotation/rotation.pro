CONFIG		+= qtopiaapp

HEADERS		= rotation.h
SOURCES		= rotation.cpp main.cpp
INTERFACES	= rotationsettingsbase.ui
TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= rotation 

help.files=$${QTOPIA_DEPOT_PATH}/help/html/rotation.html
help.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Rotation.desktop
desktop.path=/apps/Settings
pics.files=$${QTOPIA_DEPOT_PATH}/pics/rotation/*
pics.path=/pics/rotation
INSTALLS	+= desktop help
PICS_INSTALLS+=pics

