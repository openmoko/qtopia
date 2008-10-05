!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_ui_components_calltypeselector

requires(enable_telephony)

depends(server/core_server)
depends(server/phone/telephony/callpolicymanager/abstract)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(calltypeselector.pri)
