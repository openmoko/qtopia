CONFIG	    += qtopiaapp

HEADERS	    = buttoneditordialog.h  

SOURCES	    = buttoneditordialog.cpp main.cpp

INTERFACES  = buttonrow.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}
TARGET	    = buttoneditor

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=buttoneditor.html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/buttoneditor/*
pics.path=/pics/buttoneditor
buttonpics.files=$${QTOPIA_DEPOT_PATH}/pics/Button/*
buttonpics.path=/pics/Button/
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Buttons.desktop
desktop.path=/apps/Settings/
INSTALLS+=desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics buttonpics

PACKAGE_DESCRIPTION=Hardware buttons editor for the Qtopia environment.
