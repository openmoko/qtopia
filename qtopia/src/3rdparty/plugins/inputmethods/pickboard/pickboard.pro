CONFIG		+= qtopiaplugin
TARGET		= qpickboard

HEADERS		= pickboard.h pickboardcfg.h pickboardimpl.h pickboardpicks.h
SOURCES		= pickboard.cpp pickboardcfg.cpp pickboardimpl.cpp pickboardpicks.cpp

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/inputmethods/qpickboard.desktop
desktop.path=/plugins/inputmethods/
pics.files=$${QTOPIA_DEPOT_PATH}/pics/pickboard/*
pics.path=/pics/pickboard

INSTALLS+=desktop
PICS_INSTALLS+=pics

TRANSLATABLES = $${HEADERS} $${SOURCES}

PACKAGE_NAME	= qpe-pickboard

