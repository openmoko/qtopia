!qbuild{
qtopia_project(server_lib)
TARGET=qpe_infrared_obexpush

requires(enable_infrared)
requires(contains(PROJECTS,server/infrared/powermgr))

depends(server/core_server)
depends(server/comm/obex)
depends(server/comm/filetransfer)
depends(libraries/qtopiacomm/ir)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(obexpush.pri)
