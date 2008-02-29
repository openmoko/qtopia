CONFIG		+= qtopiaapp

HEADERS		= fifteen.h
SOURCES		= fifteen.cpp main.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

TARGET		= fifteen

desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/fifteen.desktop
desktop.path=/apps/Games
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=fifteen.html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/fifteen/*
pics.path=/pics/fifteen
INSTALLS+=desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Try to get the fifteen pieces in order by sliding them around.
