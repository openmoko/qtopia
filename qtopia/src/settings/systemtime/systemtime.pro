CONFIG		+= qtopiaapp

HEADERS		= settime.h
SOURCES		= settime.cpp main.cpp
TRANSLATABLES = $${HEADERS} $${SOURCES} 
TARGET		= systemtime

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=systemtime*

timeservice.files+=$${QTOPIA_DEPOT_PATH}/services/Time/systemtime
dateservice.files+=$${QTOPIA_DEPOT_PATH}/services/Date/systemtime
timeservice.path=/services/Time
dateservice.path=/services/Date
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/datetime.desktop
desktop.path=/apps/Settings
pics.files=$${QTOPIA_DEPOT_PATH}/pics/systemtime/*
pics.path=/pics/systemtime
INSTALLS	+= desktop dateservice timeservice
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Date/time setting dialog for the Qtopia environment.
