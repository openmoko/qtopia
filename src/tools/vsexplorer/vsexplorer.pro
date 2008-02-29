qtopia_project(qtopia app)
TARGET=vsexplorer
CONFIG+=no_tr

SOURCES		= vsexplorer.cpp

#enable_readline {
#    DEFINES += USE_READLINE
#    LIBS += -lreadline
#}

pkg.desc=Value Space explorer for Qtopia
pkg.domain=base
