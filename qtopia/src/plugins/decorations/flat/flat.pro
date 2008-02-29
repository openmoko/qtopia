CONFIG		+= qtopiaplugin

TARGET		= flat

HEADERS		= flat.h
SOURCES		= flat.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

pics.files=$${QTOPIA_DEPOT_PATH}/pics/decorations/flat/*
pics.path=/pics/decorations/flat
PICS_INSTALLS+=pics
