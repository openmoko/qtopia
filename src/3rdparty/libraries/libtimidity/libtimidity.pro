qtopia_project(external lib)

TARGET      =   timidity
VERSION     =   1.0.0
CONFIG-=warn_on

HEADERS     = src/common.h \
                src/dls1.h \
                src/dls2.h \
                src/instrum.h \
                src/instrum_dls.h \
                src/mix.h \
                src/options.h \
                src/output.h \
                src/playmidi.h \
                src/readmidi.h \
                src/resample.h \
                src/tables.h \
                src/timidity.h \
                src/timidity_internal.h

SOURCES = src/common.c \
            src/instrum.c \
            src/instrum_dls.c \
            src/mix.c \
            src/output.c \
            src/playmidi.c \
            src/readmidi.c \
            src/resample.c \
            src/stream.c \
            src/tables.c \
            src/timidity.c

pkg.desc=Timidity MIDI library
pkg.domain=lib

patches.files=$$QTOPIA_DEPOT_PATH/src/3rdparty/libraries/libtimidity/config/*
patches.path=/etc/timidity
patches.hint=config files

timidityconfig.commands=$$COMMAND_PREFIX\
    cat $$PWD/timidity.cfg | sed 's:QTOPIA_PREFIX:$$QTOPIA_PREFIX:' >$(INSTALL_ROOT)/etc/timidity/timidity.cfg
timidityconfig.path=/etc/timidity


INSTALLS+=patches timidityconfig

# FIXME "make syncqtopia"
dep(INCLUDEPATH+=$$PWD/src)
idep(LIBS+=-l$$TARGET)

