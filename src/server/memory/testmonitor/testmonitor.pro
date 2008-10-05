!qbuild{
qtopia_project(server_lib)
TARGET=qpe_memory_testmonitor

depends(server/core_server)
depends(server/memory/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(testmonitor.pri)
