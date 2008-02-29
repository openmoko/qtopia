CONFIG		+= qtopiaapp
requires(QTOPIA_PDA)

HEADERS		= launchersettings.h wallpaperselector.h 
SOURCES		= launchersettings.cpp wallpaperselector.cpp  main.cpp
INTERFACES	= launchersettingsbase.ui wallpaperselectorbase.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}
TARGET		= launchersettings


help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=launchersettings*
pics.files=$${QTOPIA_DEPOT_PATH}/pics/launchersettings/*
pics.path=/pics/launchersettings
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Launcher.desktop
desktop.path=/apps/Settings
INSTALLS+=desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Launcher settings dialog for the Qtopia environment.
