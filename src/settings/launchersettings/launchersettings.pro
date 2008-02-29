CONFIG		+= qtopiaapp
requires(QTOPIA_PDA)

HEADERS		= launchersettings.h wallpaperselector.h 
SOURCES		= launchersettings.cpp wallpaperselector.cpp  main.cpp
INTERFACES	= launchersettingsbase.ui wallpaperselectorbase.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}
TARGET		= launchersettings


help.files=$${QTOPIA_DEPOT_PATH}/help/html/launchersettings*
help.path=/help/html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/launchersettings/*
pics.path=/pics/launchersettings
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Launcher.desktop
desktop.path=/apps/Settings
INSTALLS+=desktop help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Launcher settings dialog for the Qtopia environment.
