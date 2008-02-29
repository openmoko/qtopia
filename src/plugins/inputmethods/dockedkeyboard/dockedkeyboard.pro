qtopia_project(qtopia plugin)
TARGET=qdockedkeyboard
CONFIG+=no_singleexec
#FIXME extract to a library or something
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/server

HEADERS		= dockedkeyboard.h \
		    dockedkeyboardimpl.h
SOURCES		= dockedkeyboard.cpp\
		    dockedkeyboardimpl.cpp

KEYBOARDHEADERS = ../keyboard/keyboard.h \
		    ../keyboard/pickboardcfg.h \
		    ../keyboard/pickboardpicks.h \
                    ../keyboard/keyboardframe.h

KEYBOARDSOURCES =  ../keyboard/keyboard.cpp \
		    ../keyboard/pickboardcfg.cpp \
		    ../keyboard/pickboardpicks.cpp \
                    ../keyboard/keyboardframe.cpp

HEADERS+=$$KEYBOARDHEADERS
SOURCES+=$$KEYBOARDSOURCES

pics.files=$${QTOPIA_DEPOT_PATH}/pics/keyboard/*.png
pics.path=/pics/keyboard

#INSTALLS += pics //already installed by keyboard
 
pkg.name=qpe-dockedkeyboard
pkg.domain=libs
