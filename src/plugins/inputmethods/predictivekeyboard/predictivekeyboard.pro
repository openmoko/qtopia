qtopia_project(qtopia plugin)
TARGET=qpredictivekeyboard
CONFIG+=no_singleexec

HEADERS		= predictivekeyboard.h \
		  predictivekeyboardimpl.h\
                  pred.h \
                  predkeyboard.h \
                  proxyscreen.h

SOURCES		= predictivekeyboard.cpp \
		  predictivekeyboardimpl.cpp\
                  pred.cpp \
                  predkeyboard.cpp \
                  proxyscreen.cpp
x11 {
    depends(libraries/qtopiainputmethod)
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
