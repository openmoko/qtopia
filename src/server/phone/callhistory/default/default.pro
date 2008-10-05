!qbuild{

requires(enable_telephony)
qtopia_project(server_lib)
TARGET=qpe_phone_callhistory_default

depends(server/core_server)
depends(server/phone/callhistory/abstract)
depends(server/phone/ui/callcontactmodelview)
depends(server/phone/telephony/dialercontrol)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

callhistorydesktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/callhistory.desktop
callhistorydesktop.path=/apps/Applications
callhistorydesktop.hint=desktop
INSTALLS+=callhistorydesktop

callhistorypics.files=$$QTOPIA_DEPOT_PATH/pics/callhistory/*
callhistorypics.path=/pics/callhistory
callhistorypics.hint=pics
INSTALLS+=callhistorypics

callhistoryservice.files=$$QTOPIA_DEPOT_PATH/services/CallHistory/qpe
callhistoryservice.path=/services/CallHistory
INSTALLS+=callhistoryservice

include(default.pri)
