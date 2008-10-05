qbuild{
SOURCEPATH+=usb
} else {
PREFIX=USB
VPATH+=usb
}

USB_HEADERS+=\
    qusbmanager.h \
    qusbgadget.h \
    qusbethernetgadget.h \
    qusbserialgadget.h \
    qusbstoragegadget.h

USB_SOURCES+=\
    qusbmanager.cpp \
    qusbgadget.cpp \
    qusbethernetgadget.cpp \
    qusbserialgadget.cpp \
    qusbstoragegadget.cpp

qbuild{
HEADERS+=$$USB_HEADERS
SOURCES+=$$USB_SOURCES
} else {
sdk_usb_headers.files=$$USB_HEADERS
sdk_usb_headers.path=/include/qtopia/comm
sdk_usb_headers.hint=sdk headers
INSTALLS+=sdk_usb_headers
}
