CONFIG		+= qtopiaapp

HEADERS		= appservices.h
SOURCES		= appservices.cpp main.cpp

INTERFACES	= appservicesbase.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= appservices


pics.files=$${QTOPIA_DEPOT_PATH}/pics/appservices/*
pics.path=/pics/appservices
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/AppServices.desktop
desktop.path=/apps/Settings
INSTALLS+=desktop
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Allows you to choose which application provides which service.
