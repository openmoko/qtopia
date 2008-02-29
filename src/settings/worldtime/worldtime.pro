CONFIG		+= qtopiaapp

UI_HEADERS_DIR	= $${UI_HEADERS_DIR}/qtopia

HEADERS		= worldtime.h zonemap.h sun.h stylusnormalizer.h cityinfo.h
SOURCES		= worldtime.cpp zonemap.cpp sun.cpp stylusnormalizer.cpp main.cpp cityinfo.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}
TARGET		= worldtime


help.files=$${QTOPIA_DEPOT_PATH}/help/html/worldtime*
help.path=/help/html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/worldtime/*
pics.path=/pics/worldtime
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/worldtime.desktop
desktop.path=/apps/Applications
INSTALLS+=desktop help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=The time-zone manager for the Qtopia environment.
