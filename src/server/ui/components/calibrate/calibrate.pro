!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_components_calibrate

requires(!x11)

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(calibrate.pri)

calibratedesktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/calibrate.desktop
calibratedesktop.path=/apps/Settings
calibratedesktop.hint=desktop
INSTALLS+=calibratedesktop

calibrateservice.files=$$QTOPIA_DEPOT_PATH/services/calibrate/calibrate
calibrateservice.path=/services/calibrate
INSTALLS+=calibrateservice

calibratepics.files=$$QTOPIA_DEPOT_PATH/pics/calibrate/*
calibratepics.path=/pics/calibrate
calibratepics.hint=pics
INSTALLS+=calibratepics

sdk_calibrate_headers.files=$$HEADERS
sdk_calibrate_headers.path=/src/server/ui/components/calibrate
sdk_calibrate_headers.hint=sdk
INSTALLS+=sdk_calibrate_headers

