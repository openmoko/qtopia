!qbuild{
qtopia_project(desktop external app)
TARGET=micro_httpd
license(BSD GPL_COMPATIBLE)
LIBS=-lc
QMAKE_LIBS_DYNLOAD=
QMAKE_CFLAGS_RELEASE=-Os
QMAKE_LFLAGS_RELEASE=-nodefaultlibs
}

SOURCES=micro_httpd.c

