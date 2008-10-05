qbuild {
SOURCEPATH+=bluetooth
QT+=dbus
} else {
PREFIX=BLUETOOTH
VPATH+=bluetooth
}

BLUETOOTH_HEADERS+=\
    qbluetoothaddress.h\
    qbluetoothlocaldevice.h\
    qbluetoothlocaldevicemanager.h\
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
    qbluetoothsdpquery.h\
    qbluetoothsdprecord.h\
    qbluetoothsdpuuid.h\
    qbluetoothremotedevicedialog.h\
    qbluetoothrfcommserialport.h \
    qbluetoothabstractservice.h \
    qbluetoothservicecontroller.h \
    qbluetoothaudiogateway.h \
    qbluetoothl2capsocket.h \
    qbluetoothl2capserver.h \
    qbluetoothl2capdatagramsocket.h \
    qbluetoothglobal.h \
    qbluetoothauthorizationagent.h

BLUETOOTH_SOURCES+=\
    qbluetoothaddress.cpp\
    qbluetoothlocaldevice.cpp\
    qbluetoothlocaldevicemanager.cpp\
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
    qbluetoothsdpquery.cpp\
    qbluetoothsdprecord.cpp\
    qbluetoothsdpuuid.cpp\
    qbluetoothremotedeviceselector.cpp \
    qbluetoothremotedevicedialog.cpp\
    qbluetoothrfcommserialport.cpp \
    qbluetoothabstractservice.cpp \
    qbluetoothservicecontroller.cpp \
    qbluetoothaudiogateway.cpp \
    qsdpxmlparser.cpp \
    qsdpxmlgenerator.cpp \
    qbluetoothl2capsocket.cpp \
    qbluetoothl2capserver.cpp \
    qbluetoothl2capdatagramsocket.cpp \
    qbluetoothauthorizationagent.cpp

BLUETOOTH_PRIVATE_HEADERS+=\
    qbluetoothremotedevicedialog_p.h \
    qsdpxmlgenerator_p.h \
    qbluetoothabstractsocket_p.h \
    qbluetoothabstractserver_p.h

BLUETOOTH_SEMI_PRIVATE_HEADERS+=\
    qbluetoothnamespace_p.h\
    qsdpxmlparser_p.h \
    qbluetoothremotedeviceselector_p.h \

BLUETOOTH_UNIX.TYPE=CONDITIONAL_SOURCES
BLUETOOTH_UNIX.CONDITION=unix
BLUETOOTH_UNIX.SOURCES=qbluetoothsocketengine_unix.cpp
!qbuild:BLUETOOTH_UNIX.PREFIX=BLUETOOTH
!qbuild:CONDITIONAL_SOURCES(BLUETOOTH_UNIX)

qbuild {
HEADERS+=$$BLUETOOTH_HEADERS
SOURCES+=$$BLUETOOTH_SOURCES
PRIVATE_HEADERS+=$$BLUETOOTH_PRIVATE_HEADERS
SEMI_PRIVATE_HEADERS+=$$BLUETOOTH_SEMI_PRIVATE_HEADERS
} else {
sdk_bluetooth_headers.files=$$BLUETOOTH_HEADERS
sdk_bluetooth_headers.path=/include/qtopia/comm
sdk_bluetooth_headers.hint=sdk headers
INSTALLS+=sdk_bluetooth_headers

sdk_bluetooth_private_headers.files=$$BLUETOOTH_SEMI_PRIVATE_HEADERS
sdk_bluetooth_private_headers.path=/include/qtopia/comm/private
sdk_bluetooth_private_headers.hint=sdk headers
INSTALLS+=sdk_bluetooth_private_headers
}

pics.files=$$QTOPIA_DEPOT_PATH/pics/bluetooth/*
pics.path=/pics/bluetooth
pics.hint=pics
INSTALLS+=pics

