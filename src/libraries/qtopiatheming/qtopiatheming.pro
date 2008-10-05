!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiatheming
}

include(qtopiatheming.pri)

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/theming
headers.hint=sdk headers
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
