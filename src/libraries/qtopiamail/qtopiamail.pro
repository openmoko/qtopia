qtopia_project(qtopia lib)
TARGET=qtopiamail

CONFIG+=no_tr qtopia_visibility

HEADERS+=\
    emailfolderaccess.h\
    mailmessage.h\
    qtopiamail.h\
    base64stream.h\
    qpstream.h \
    longstring.h \
    longstream.h

SOURCES+=\
    emailfolderaccess.cpp\
    mailmessage.cpp\
    base64stream.cpp\
    qpstream.cpp \
    longstring.cpp \
    longstream.cpp

sdk_qtopiamail_headers.files=$${HEADERS}
sdk_qtopiamail_headers.path=/include/qtopia/mail
sdk_qtopiamail_headers.hint=sdk headers
INSTALLS+=sdk_qtopiamail_headers

pkg.desc=Allows access to the Qtopia mailbox files.
pkg.domain=net,localnet,pim

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
