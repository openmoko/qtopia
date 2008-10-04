build_qtopia_sqlite {

!qbuild{
qtopia_project(external lib)
license(FREEWARE)
TARGET=qtopia-sqlite
CONFIG+=syncqtopia
CONFIG(release,debug|release):DEFINES*=NDEBUG

sourcedir=$$QTE_DEPOT_PATH/src/3rdparty/sqlite
INCLUDEPATH += $$sourcedir
VPATH += $$sourcedir

# I'd do a depend but we only want defines, not symbols
INCLUDEPATH+=$$QTDIR/include/QtCore
}

DEFINES+=SQLITE_OMIT_LOAD_EXTENSION SQLITE_CORE
# QMAKE_CFLAGS+=-O3
# QMAKE_CXXFLAGS+=-O3

HEADERS=\
    sqlite3.h

SOURCES=\
    sqlite3.c

!qbuild{
headers.files=$$HEADERS
CONFIG+=syncqtopia
headers.path=/include/sqlite
headers.hint=non_qt_headers
INSTALLS+=headers

qt_inc(sqlite)
idep(LIBS+=-l$$TARGET)
}

} else {
qtopia_project(stub)
idep(LIBS+=-lsqlite3)
}
