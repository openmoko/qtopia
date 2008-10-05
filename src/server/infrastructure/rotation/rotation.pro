!qbuild{
qtopia_project(server_lib)
TARGET=qpe_infrastructure_rotation

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(rotation.pri)

qtopiarotationmanager.files=$$QTOPIA_DEPOT_PATH/services/RotationManager/qpe
qtopiarotationmanager.path=/services/RotationManager
INSTALLS+=qtopiarotationmanager
