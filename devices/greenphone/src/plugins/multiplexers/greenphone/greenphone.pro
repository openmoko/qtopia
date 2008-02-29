qtopia_project(stub)
TARGET=greenphonemultiplex

file=$$dirname(QTOPIA_LIBQTOPIAPHONE)/../devices/greenphone/src/plugins/multiplexers/greenphone/lib$${TARGET}.so
!exists($$file):error(Cannot locate lib$${TARGET}.so. You must copy this file to the image for full functionality.)

# install the plugin
path=/plugins/multiplexers
var=lib_$$TARGET
commands=\
    install -c $$LITERAL_QUOTE$$file$$LITERAL_QUOTE $$LITERAL_QUOTE$(INSTALL_ROOT)$$path/$$tail($$file)$$LITERAL_QUOTE
CONFIG(release,debug|release):!isEmpty(QMAKE_STRIP):commands+=\
    $$LINE_SEP_VERBOSE $$QMAKE_STRIP $$QMAKE_STRIPFLAGS_LIB $$LITERAL_QUOTE$(INSTALL_ROOT)$$path/$$tail($$file)$$LITERAL_QUOTE
eval($${var}.commands=\$$commands)
eval($${var}.path=$$path)
echo(installing $$var)
INSTALLS+=$$var
