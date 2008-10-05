!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_core_server
}

x11:MODULES*=Xtst
include(core_server.pro)
