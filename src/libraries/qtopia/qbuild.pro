TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=qtopia
VERSION=4.0.0

MODULE_NAME=qtopia
QTOPIA=base gfx
enable_sxe:QTOPIA*=security
QT*=sql xml svg
MODULES*=md5 realtime sqlite zlib
CONFIG+=i18n use_default_i18n_rules

DEFINES+=QTOPIA_PAGE_SIZE=$$define_value($$QTOPIA_PAGE_SIZE)
DEFINES+=QTOPIA_PAGE_MASK=$$define_value($$QTOPIA_PAGE_MASK)

MOC_COMPILE_EXCEPTIONS+=qtopiainputdialog_p.h

include(qtopia.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

