qtopia_project(desktop external app)
include(../../libraries/sqlite/sqlite.pro)
CONFIG-=syncqtopia
INSTALLS-=sdk_headers
TARGET=sqlite

SOURCES+=\
    shell.c\
    complete.c
LIBS+=-lpthread

