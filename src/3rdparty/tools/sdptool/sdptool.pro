qtopia_project(external app)
license(GPL)
TARGET=sdptool

DEFINES += VERSION=$${LITERAL_ESCAPED_QUOTE}3.1$$LITERAL_ESCAPED_QUOTE

CONFIG+=gpl_ok

INCLUDEPATH+=../../libraries/bluez

SOURCES=sdptool.c

depends(3rdparty/libraries/bluez)

