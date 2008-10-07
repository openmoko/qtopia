qtopia_project(qtopia app)
!x11:qtopia_project(qtopia plugin)
TARGET=qkeyboard
CONFIG+=no_singleexec

HEADERS		= keyboard.h \
		    pickboardcfg.h \
		    pickboardpicks.h \
                    keyboardframe.h
SOURCES		= keyboard.cpp \
		    pickboardcfg.cpp \
		    pickboardpicks.cpp \
                    keyboardframe.cpp

x11 {
    depends(libraries/qtopiainputmethod)
}

!x11 {
    HEADERS += keyboardimpl.h
    SOURCES += keyboardimpl.cpp
}

pics.files=$${QTOPIA_DEPOT_PATH}/pics/keyboard/*
pics.path=/pics/keyboard
pics.hint=pics
INSTALLS += pics
 
pkg.name=qpe-keyboard
pkg.domain=trusted
