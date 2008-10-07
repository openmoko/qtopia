qtopia_project(qtopia app)
!x11:qtopia_project(qtopia plugin)
TARGET=qpredictivekeyboard
CONFIG+=no_singleexec

HEADERS		= predictivekeyboard.h \
                  pred.h \
                  predkeyboard.h \
                  proxyscreen.h

SOURCES		= predictivekeyboard.cpp \
                  pred.cpp \
                  predkeyboard.cpp \
                  proxyscreen.cpp
x11 {
    depends(libraries/qtopiainputmethod)
}

!x11 {
    HEADERS += predictivekeyboardimpl.h
    SOURCES += predictivekeyboardimpl.cpp
}


pkg.name=qpe-predictivekeyboard
pkg.domain=trusted

dict.files=
dict.files=words.qtrie
dict.path=/etc
INSTALLS += dict

pics.files=$$QTOPIA_DEPOT_PATH/pics/predictivekeyboard/*
pics.path=/pics/predictivekeyboard
pics.hint=pics
INSTALLS+=pics
