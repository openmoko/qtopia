!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_shutdown

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(shutdown.pri)

#enable this if a shutdown application entry in the app launcher list is required
#quitdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/quit.desktop
#quitdesktop.path=/apps/Settings
#quitdesktop.hint=desktop
#INSTALLS+=quitdesktop

