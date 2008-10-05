!qbuild{
qtopia_project(server_lib)
TARGET=qpe_media_alertservice

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(alertservice.pri)

alertservice.files=$$QTOPIA_DEPOT_PATH/services/Alert/qpe
alertservice.path=/services/Alert
INSTALLS+=alertservice

