!qbuild{
qtopia_project(qtopia lib)
TARGET=inputmatch
license(FREEWARE)

CONFIG+=no_tr
}

HEADERS=\
    pkimmatcher.h\
    symbolpicker.h\
    wordpicker.h\
    picker.h\
    inputmatchglobal.h\

SOURCES=\
    pkimmatcher.cpp\
    symbolpicker.cpp\
    wordpicker.cpp\
    picker.cpp\

!qbuild{
headers.files=$$HEADERS
CONFIG+=syncqtopia
headers.path=/include/qtopia/inputmatch
headers.hint=headers sdk
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
