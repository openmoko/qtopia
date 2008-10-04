!qbuild{
qtopia_project(external app)
CONFIG-=qt
TARGET=sxe_policy_runner
# Pull in only libc
LIBS=-lc
QMAKE_LIBS_DYNLOAD=
QMAKE_CFLAGS_RELEASE=-Os
QMAKE_LFLAGS_RELEASE=-nodefaultlibs
}

SOURCES=main.c

