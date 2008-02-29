qtopia_project(qtopia plugin)
TARGET=qkeyboard
CONFIG+=no_singleexec

HEADERS		= keyboard.h \
		    pickboardcfg.h \
		    pickboardpicks.h \
		    keyboardimpl.h\
                    keyboardframe.h
SOURCES		= keyboard.cpp \
		    pickboardcfg.cpp \
		    pickboardpicks.cpp \
		    keyboardimpl.cpp\
                    keyboardframe.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/inputmethods/qkeyboard.desktop
desktop.path=/plugins/inputmethods/
desktop.hint=desktop
INSTALLS+=desktop

pics.files=$${QTOPIA_DEPOT_PATH}/pics/keyboard/*.png
pics.path=/pics/keyboard

INSTALLS += pics
 
pkg.name=qpe-keyboard
pkg.domain=libs
