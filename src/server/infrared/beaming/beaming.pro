!qbuild{
qtopia_project(server_lib)
TARGET=qpe_infrared_beaming

requires(enable_infrared)
requires(contains(PROJECTS,server/infrared/powermgr))

depends(server/core_server)
depends(server/comm/obex)
depends(server/comm/filetransfer)
depends(libraries/qtopiacomm/ir)
depends(libraries/qtopiapim)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(beaming.pri)

irbeamingservice.files=$$QTOPIA_DEPOT_PATH/services/InfraredBeaming/qpe
irbeamingservice.path=/services/InfraredBeaming
INSTALLS+=irbeamingservice

irqdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/InfraredBeaming
irqdsservice.path=/etc/qds
INSTALLS+=irqdsservice

