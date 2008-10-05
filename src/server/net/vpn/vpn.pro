!qbuild{
qtopia_project(server_lib)
TARGET=qpe_net_vpn

requires(enable_vpn)

depends(libraries/qtopiacomm/vpn)
depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(vpn.pri)
