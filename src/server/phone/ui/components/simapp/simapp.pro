!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_ui_components_simapp

requires(enable_cell)

depends(server/core_server)
depends(server/infrastructure/messageboard)
depends(server/processctrl/appmonitor)
depends(libraries/qtopiaphone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

simappdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/simapp.desktop
simappdesktop.path=/apps/Applications
simappdesktop.hint=desktop
INSTALLS+=simappdesktop

simapppics.files=$$QTOPIA_DEPOT_PATH/pics/simapp/*
simapppics.path=/pics/simapp
simapppics.hint=pics
INSTALLS+=simapppics

simapphelp.source=$$QTOPIA_DEPOT_PATH/help
simapphelp.files=\
    simapp.html
simapphelp.hint=help
INSTALLS+=simapphelp


include(simapp.pri)
