TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=qtopiabase
VERSION=4.0.0

MODULE_NAME=qtopiabase
QTOPIA=
MODULES*=mathlib
# Included directly because it's not a module
DEPENDS*=/src/3rdparty/libraries/dlmalloc::persisted
enable_dbus_ipc:QT*=dbus
CONFIG+=i18n use_default_i18n_rules

# We need to prevent some files from appearing in TRANSLATABLES
TRANSLATABLES*=$$FORMS $$HEADERS $$PRIVATE_HEADERS $$SOURCES
CONFIG+=no_auto_translatables

HEADERS+=$$path(custom-qtopia.h,generated)
MOC_IGNORE+=$$path(custom-qtopia.h,generated)
SOURCES+=$$path(custom-qtopia.cpp,generated)

include(qtopiabase.pro)

x11:SOURCEPATH+=/qtopiacore/qt/src/gui/embedded
# If we do this QUnixSocket and QTransportAuth stuff is hidden from Qtopia
x11:CONFIG-=hide_symbols

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

