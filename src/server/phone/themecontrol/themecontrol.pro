!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_themecontrol

depends(server/core_server)
depends(libraries/qtopiatheming)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}
    
include(themecontrol.pri)

defaulttheme.files=$$QTOPIA_DEPOT_PATH/etc/themes/default/*
defaulttheme.path=/etc/themes/default
INSTALLS+=defaulttheme

defaultpics.files=$$QTOPIA_DEPOT_PATH/pics/themes/default*
defaultpics.path=/pics/themes
defaultpics.hint=pics
INSTALLS+=defaultpics
