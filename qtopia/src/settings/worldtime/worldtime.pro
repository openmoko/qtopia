CONFIG		+= qtopiaapp

UI_HEADERS_DIR	= $${UI_HEADERS_DIR}/qtopia

HEADERS		= worldtime.h zonemap.h sun.h stylusnormalizer.h cityinfo.h
SOURCES		= worldtime.cpp zonemap.cpp sun.cpp stylusnormalizer.cpp main.cpp cityinfo.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}
TARGET		= worldtime


help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=worldtime*
pics.files=$${QTOPIA_DEPOT_PATH}/pics/worldtime/*
pics.path=/pics/worldtime
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/worldtime.desktop
desktop.path=/apps/Applications
INSTALLS+=desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=The time-zone manager for the Qtopia environment.
