qtopia_project(external lib)
license(GPL)
TARGET=bluetooth
qtopia_depot:CONFIG+=staticlib
idep(CONFIG-=enable_singleexec,CONFIG)

HEADERS=\
    bluetooth/bluetooth.h\
    bluetooth/bnep.h\
    bluetooth/cmtp.h\
    bluetooth/hci.h\
    bluetooth/hci_lib.h\
    bluetooth/hidp.h\
    bluetooth/l2cap.h\
    bluetooth/rfcomm.h\
    bluetooth/sco.h\
    bluetooth/sdp.h\
    bluetooth/sdp_lib.h

SOURCES=\
    bluetooth.c\
    hci.c\
    sdp.c

sdk_headers.files=$$HEADERS
sdk_headers.path=/include/bluetooth
sdk_headers.hint=non_qt_headers
INSTALLS+=sdk_headers

idep(!gpl_ok:error($$TARGET is licenced under the GPL))
idep(LIBS+=-l$$TARGET)
