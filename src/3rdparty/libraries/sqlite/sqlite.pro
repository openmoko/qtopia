qtopia_project(external lib)
license(FREEWARE)
TARGET=qtopia-sqlite
CONFIG+=syncqtopia
CONFIG(release,debug|release):DEFINES*=NDEBUG

sourcedir=$$QTE_DEPOT_PATH/src/3rdparty/sqlite
INCLUDEPATH += $$sourcedir
VPATH += $$sourcedir

SOURCES += alter.c \
           analyze.c \
           attach.c \
           auth.c \
           btree.c \
           build.c \
           callback.c \
           date.c \
           delete.c \
           expr.c \
           func.c \
           hash.c \
           insert.c \
           legacy.c \
           main.c \
           opcodes.c \
           os.c \
           pager.c \
           parse.c \
           pragma.c \
           prepare.c \
           printf.c \
           random.c \
           select.c \
           table.c \
           tokenize.c \
           trigger.c \
           update.c \
           utf.c \
           util.c \
           vacuum.c \
           vdbeapi.c \
           vdbeaux.c \
           vdbe.c \
           vdbefifo.c \
           vdbemem.c \
           where.c

unix:SOURCES += os_unix.c
win32:SOURCES += os_win.c

HEADERS+= btree.h \
          hash.h \
          keywordhash.h \
          opcodes.h \
          os.h \
          os_common.h \
          pager.h \
          parse.h \
          sqlite3.h \
          sqliteInt.h \
          vdbe.h \
          vdbeInt.h

sdk_headers.files=$$HEADERS
sdk_headers.path=/include/sqlite
sdk_headers.hint=non_qt_headers
INSTALLS+=sdk_headers

idep(LIBS+=-l$$TARGET)
idep(INCLUDEPATH+=$$sourcedir)

