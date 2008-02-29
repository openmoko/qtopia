CONFIG		+= qtopiaplugin

TARGET		= qunikeyboard

HEADERS		= unikeyboard.h unikeyboardimpl.h
SOURCES		= unikeyboard.cpp unikeyboardimpl.cpp

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/inputmethods/qunikeyboard.desktop
desktop.path=/plugins/inputmethods/

INSTALLS+=desktop

TRANSLATABLES   = $${HEADERS} $${SOURCES}
PACKAGE_NAME	= qpe-unikeyboard

