!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_launcherviews_archiveview

requires(drmagent)

depends(server/core_server)
depends(server/ui/launcherviews/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(archiveview.pri)

archivesdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/archives.desktop
archivesdesktop.path=/apps/Applications
archivesdesktop.hint=desktop
INSTALLS+=archivesdesktop

archiveshelp.source=$$QTOPIA_DEPOT_PATH/help
archiveshelp.files=qpe-archives*
archiveshelp.hint=help
INSTALLS+=archiveshelp

archivespics.files=$$QTOPIA_DEPOT_PATH/pics/archives/*
archivespics.path=/pics/archives
archivespics.hint=pics
INSTALLS+=archivespics

