CONFIG		+= qtopiaapp

HEADERS		= light.h
SOURCES		= light.cpp main.cpp
INTERFACES	= lightsettingsbase.ui 

PHONE_INTERFACES = displaysettingsbase.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES} $${PHONE_INTERFACES}

QTOPIA_PHONE{
    INTERFACES += $${PHONE_INTERFACES}
}


TARGET		= light-and-power


help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=light-and-power*
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Light.desktop
desktop.path=/apps/Settings
pics.files=$${QTOPIA_DEPOT_PATH}/pics/light-and-power/*
pics.path=/pics/light-and-power
INSTALLS	+= desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Light and Power settings dialog for the Qtopia environment.
