CONFIG		+= qtopiaapp

HEADERS		= fifteen.h
SOURCES		= fifteen.cpp main.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

TARGET		= fifteen

desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/fifteen.desktop
desktop.path=/apps/Games
help.files=$${QTOPIA_DEPOT_PATH}/help/html/fifteen.html
help.path=/help/html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/fifteen/*
pics.path=/pics/fifteen
INSTALLS+=desktop help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Try to get the fifteen pieces in order by sliding them around.
