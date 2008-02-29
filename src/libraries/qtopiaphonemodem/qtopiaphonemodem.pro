qtopia_project(stub)
TARGET=qtopiaphonemodem
idep(LIBS+=$$QTOPIA_LIBQTOPIAPHONEMODEM)
depends(libraries/qtopiaphone)

# install libqtopiaphonemodem.so
var=lib_$$TARGET
commands=\
    install -c $$LITERAL_QUOTE$${QTOPIA_LIBQTOPIAPHONEMODEM}.$$QTOPIA_LIBQTOPIAPHONEMODEM_VERSION$$LITERAL_QUOTE $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/libqtopiaphonemodem.so.$$QTOPIA_LIBQTOPIAPHONEMODEM_VERSION$$LITERAL_QUOTE $$LINE_SEP_VERBOSE\
    ln -sf libqtopiaphonemodem.so.$$QTOPIA_LIBQTOPIAPHONEMODEM_VERSION $(INSTALL_ROOT)$$libdir/libqtopiaphonemodem.so $$LINE_SEP_VERBOSE\
    ln -sf libqtopiaphonemodem.so.$$QTOPIA_LIBQTOPIAPHONEMODEM_VERSION $(INSTALL_ROOT)$$libdir/libqtopiaphonemodem.so.$$QTOPIA_LIBQTOPIAPHONEMODEM_MAJOR_VERSION $$LINE_SEP_VERBOSE\
    ln -sf libqtopiaphonemodem.so.$$QTOPIA_LIBQTOPIAPHONEMODEM_VERSION $(INSTALL_ROOT)$$libdir/libqtopiaphonemodem.so.$${QTOPIA_LIBQTOPIAPHONEMODEM_MAJOR_VERSION}.$$QTOPIA_LIBQTOPIAPHONEMODEM_MINOR_VERSION
CONFIG(release,debug|release):!isEmpty(QMAKE_STRIP):commands+=\
    $$LINE_SEP_VERBOSE $$QMAKE_STRIP $$QMAKE_STRIPFLAGS_LIB $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/libqtopiaphonemodem.so.$$QTOPIA_LIBQTOPIAPHONEMODEM_VERSION$$LITERAL_QUOTE
eval($${var}.commands=\$$commands)
eval($${var}.path=/lib)
echo(installing $$var)
INSTALLS+=$$var
