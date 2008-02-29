CONFIG		+= qtopiaplugin

TARGET		= qunikeyboard

HEADERS		= unikeyboard.h unikeyboardimpl.h
SOURCES		= unikeyboard.cpp unikeyboardimpl.cpp

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/inputmethods/qunikeyboard.desktop
desktop.path=/plugins/inputmethods/
pics.files=$${QTOPIA_DEPOT_PATH}/pics/qunikeyboard/*
pics.path=/pics/qunikeyboard

INSTALLS+=desktop
PICS_INSTALLS+=pics

TRANSLATABLES   = $${HEADERS} $${SOURCES}
PACKAGE_NAME	= qpe-unikeyboard

