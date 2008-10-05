!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_dialfilter_gsm

requires(enable_cell)

depends(server/core_server)
depends(server/phone/telephony/dialfilter/abstract)
depends(server/phone/telephony/dialercontrol)
depends(libraries/qtopiaphone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(gsm.pri)
