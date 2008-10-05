!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_launcherviews_taskmanagerview

depends(server/core_server)
depends(server/ui/launcherviews/base)
depends(server/processctrl/appmonitor)
depends(server/processctrl/taskmanagerentry)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(taskmanagerview.pri)
