!qbuild{
qtopia_project(qtopia plugin)
TARGET=qdockedkeyboard
CONFIG+=no_singleexec no_tr
#FIXME extract to a library or something
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/server/core_server
}

HEADERS		= dockedkeyboard.h \
		    dockedkeyboardimpl.h
SOURCES		= dockedkeyboard.cpp\
		    dockedkeyboardimpl.cpp

KEYBOARD.TYPE=CONDITIONAL_SOURCES
KEYBOARD.CONDITION=!enable_singleexec
KEYBOARD.HEADERS=\
    ../keyboard/keyboard.h \
    ../keyboard/pickboardcfg.h \
    ../keyboard/pickboardpicks.h \
    ../keyboard/keyboardframe.h
KEYBOARD.SOURCES=\
    ../keyboard/keyboard.cpp \
    ../keyboard/pickboardcfg.cpp \
    ../keyboard/pickboardpicks.cpp \
    ../keyboard/keyboardframe.cpp
!qbuild:CONDITIONAL_SOURCES(KEYBOARD)

pics.files=$${QTOPIA_DEPOT_PATH}/pics/keyboard/*
pics.path=/pics/keyboard
pics.hint=pics
#INSTALLS += pics //already installed by keyboard
 
pkg.name=qpe-dockedkeyboard
pkg.domain=trusted
