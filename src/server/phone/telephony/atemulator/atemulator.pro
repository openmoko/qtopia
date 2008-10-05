!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_atemulator

requires(enable_telephony)
requires(contains(PROJECTS,tools/atinterface))

depends(server/core_server)
depends(libraries/qtopiaphone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(atemulator.pri)
