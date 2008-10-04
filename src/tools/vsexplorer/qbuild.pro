TEMPLATE=app
CONFIG+=qtopia singleexec
TARGET=vsexplorer

enable_bluetooth:QTOPIA*=comm
#enable_readline {
#    MODULES*=readline
#    DEFINES+=USE_READLINE
#}

include(vsexplorer.pro)

