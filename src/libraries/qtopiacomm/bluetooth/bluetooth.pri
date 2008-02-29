PREFIX=BLUETOOTH
VPATH+=bluetooth

BLUETOOTH_HEADERS+=\
    qbluetoothaddress.h\
    qbluetoothlocaldevice.h\
    qbluetoothlocaldevicemanager.h\
    qbluetoothobexserver.h\
    qbluetoothobexsocket.h\
    qbluetoothnamespace.h\
    qbluetoothpasskeyagent.h\
    qbluetoothpasskeyrequest.h\
    qbluetoothremotedevice.h\
    qbluetoothabstractserver.h\
    qbluetoothscoserver.h\
    qbluetoothrfcommserver.h\
    qbluetoothabstractsocket.h\
    qbluetoothscosocket.h\
    qbluetoothrfcommsocket.h\
    qsdap.h\
    qsdp.h\
    qsdpservice.h\
    qsdpuuid.h\
    qbluetoothdeviceselector.h\
    qbluetoothrfcommserialport.h \
    qbluetoothservicecontrol.h \
    qbluetoothabstractservice.h \
    qbluetoothservicecontroller.h \
    qbluetoothaudiogateway.h

BLUETOOTH_SOURCES+=\
    qbluetoothaddress.cpp\
    qbluetoothlocaldevice.cpp\
    qbluetoothlocaldevicemanager.cpp\
    qbluetoothobexserver.cpp\
    qbluetoothobexsocket.cpp\
    qbluetoothnamespace.cpp\
    qbluetoothpasskeyagent.cpp\
    qbluetoothpasskeyrequest.cpp\
    qbluetoothabstractserver.cpp\
    qbluetoothscoserver.cpp\
    qbluetoothrfcommserver.cpp\
    qbluetoothabstractsocket.cpp\
    qbluetoothscosocket.cpp\
    qbluetoothrfcommsocket.cpp\
    qbluetoothremotedevice.cpp\
    qsdap.cpp\
    qsdp.cpp\
    qsdpservice.cpp\
    qsdpuuid.cpp\
    sdplexer.cpp\
    qbluetoothdeviceselector.cpp\
    qbluetoothrfcommserialport.cpp \
    qbluetoothservicecontrolserver.cpp\
    qbluetoothabstractservice.cpp \
    qbluetoothservicecontrol.cpp \
    qbluetoothservicecontroller.cpp \
    qbluetoothaudiogateway.cpp \
    qbluetoothpairingagent.cpp \
    remotedevicepropertiesdialog.cpp 

BLUETOOTH_PRIVATE_HEADERS+=\
    qbluetoothnamespace_p.h\
    sdp_tokens_p.h\
    sdplexer_p.h \
    qbluetoothdeviceselector_p.h \
    qbluetoothpairingagent_p.h \
    qbluetoothservicecontrolserver_p.h \
    remotedevicepropertiesdialog_p.h

sdk_bluetooth_headers.files=$$BLUETOOTH_HEADERS
sdk_bluetooth_headers.path=/include/qtopia/comm
sdk_bluetooth_headers.hint=sdk headers
INSTALLS+=sdk_bluetooth_headers

sdk_bluetooth_private_headers.files=$$BLUETOOTH_PRIVATE_HEADERS
sdk_bluetooth_private_headers.path=/include/qtopiacomm/private
sdk_bluetooth_private_headers.hint=sdk headers
INSTALLS+=sdk_bluetooth_private_headers

pics.files=$$QTOPIA_DEPOT_PATH/pics/bluetooth/*
pics.path=/pics/bluetooth
pics.hint=pics
INSTALLS+=pics
