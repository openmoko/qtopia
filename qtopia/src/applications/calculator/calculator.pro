CONFIG += qtopiaapp

INCLUDEPATH += ../../libraries/qtopiacalc	

LIBS+=-lqtopiacalc
#-Wl,-export-dynamic
#export-dynamic costs around 3.5kb

SOURCES+=main.cpp

TARGET = calculator

TRANSLATABLES = $${SOURCES}


desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/calculator.desktop
desktop.path=/apps/Applications
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=calculator*
pics.files=$${QTOPIA_DEPOT_PATH}/pics/calculator/*
pics.path=/pics/calculator
plugindir.files=$${QTOPIA_DEPOT_PATH}/plugins/calculator/.directory
plugindir.path=/plugins/calculator/
INSTALLS+=desktop plugindir
PICS_INSTALLS+=pics
HELP_INSTALLS+=help

