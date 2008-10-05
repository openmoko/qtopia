!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_taskmanager

depends(server/core_server)
depends(server/ui/abstractinterfaces/taskmanager)
depends(server/ui/launcherviews/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(taskmanager.pri)

taskmanagerdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/taskmanager.desktop
taskmanagerdesktop.path=/apps/Settings
taskmanagerdesktop.hint=desktop
INSTALLS+=taskmanagerdesktop

taskmanagerservice.files=$$QTOPIA_DEPOT_PATH/services/TaskManager/qpe
taskmanagerservice.path=/services/TaskManager
INSTALLS+=taskmanagerservice

favoritesservice.files=$$QTOPIA_DEPOT_PATH/services/Favorites/qpe
favoritesservice.path=/services/Favorites
INSTALLS+=favoritesservice
