!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiasmil
CONFIG+=no_tr

enable_qtopiamedia:depends(libraries/qtopiamedia)

}

HEADERS=\
    smil.h \
    system.h \
    transfer.h \
    module.h \
    element.h \
    structure.h \
    content.h \
    layout.h \
    timing.h \
    media.h 

SOURCES=\
    smil.cpp \
    system.cpp \
    transfer.cpp \
    module.cpp \
    element.cpp \
    structure.cpp \
    content.cpp \
    layout.cpp \
    timing.cpp \
    media.cpp 

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/smil
headers.hint=sdk headers
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
