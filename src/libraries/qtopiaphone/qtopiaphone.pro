qtopia_project(stub)
TARGET=qtopiaphone
idep(LIBS+=$$QTOPIA_LIBQTOPIAPHONE)
depends(libraries/qtopiacomm/serial)
!enable_qtopiabase:depends(libraries/qtopiail)

# install libqtopiaphone.so
var=lib_$$TARGET
commands=\
    install -c $$LITERAL_QUOTE$${QTOPIA_LIBQTOPIAPHONE}.$$QTOPIA_LIBQTOPIAPHONE_VERSION$$LITERAL_QUOTE $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/libqtopiaphone.so.$$QTOPIA_LIBQTOPIAPHONE_VERSION$$LITERAL_QUOTE $$LINE_SEP_VERBOSE\
    ln -sf libqtopiaphone.so.$$QTOPIA_LIBQTOPIAPHONE_VERSION $(INSTALL_ROOT)$$libdir/libqtopiaphone.so $$LINE_SEP_VERBOSE\
    ln -sf libqtopiaphone.so.$$QTOPIA_LIBQTOPIAPHONE_VERSION $(INSTALL_ROOT)$$libdir/libqtopiaphone.so.$$QTOPIA_LIBQTOPIAPHONE_MAJOR_VERSION $$LINE_SEP_VERBOSE\
    ln -sf libqtopiaphone.so.$$QTOPIA_LIBQTOPIAPHONE_VERSION $(INSTALL_ROOT)$$libdir/libqtopiaphone.so.$${QTOPIA_LIBQTOPIAPHONE_MAJOR_VERSION}.$$QTOPIA_LIBQTOPIAPHONE_MINOR_VERSION
CONFIG(release,debug|release):!isEmpty(QMAKE_STRIP):commands+=\
    $$LINE_SEP_VERBOSE $$QMAKE_STRIP $$QMAKE_STRIPFLAGS_LIB $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/libqtopiaphone.so.$$QTOPIA_LIBQTOPIAPHONE_VERSION$$LITERAL_QUOTE
eval($${var}.commands=\$$commands)
eval($${var}.path=/lib)
echo(installing $$var)
INSTALLS+=$$var
