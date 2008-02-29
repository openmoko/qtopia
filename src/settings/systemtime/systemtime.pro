qtopia_project(qtopia app)
TARGET=systemtime
CONFIG+=qtopia_main

HEADERS		= settime.h
SOURCES		= settime.cpp main.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=systemtime*
help.hint=help
timeservice.files+=$$QTOPIA_DEPOT_PATH/services/Time/systemtime
timeservice.path=/services/Time
dateservice.files+=$$QTOPIA_DEPOT_PATH/services/Date/systemtime
dateservice.path=/services/Date
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/datetime.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/systemtime/*
pics.path=/pics/systemtime
pics.hint=pics
INSTALLS+=help timeservice dateservice desktop pics

pkg.desc=Date/time setting dialog for Qtopia.
pkg.domain=window,alarm,datetime,screensaver,launcher
