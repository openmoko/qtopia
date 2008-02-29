CONFIG		+= qtopiaplugin 

HEADERS		= keyboard.h \
		    pickboardcfg.h \
		    pickboardpicks.h \
		    keyboardimpl.h
SOURCES		= keyboard.cpp \
		    pickboardcfg.cpp \
		    pickboardpicks.cpp \
		    keyboardimpl.cpp

TARGET		= qkeyboard

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/inputmethods/qkeyboard.desktop
desktop.path=/plugins/inputmethods/

INSTALLS+=desktop
 

TRANSLATABLES= $${HEADERS} $${SOURCES}

PACKAGE_NAME = qpe-keyboard
