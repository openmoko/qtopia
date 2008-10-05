TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=qtopiamail
VERSION=4.0.0

MODULE_NAME=qtopiamail
QTOPIA*=pim
equals(QTOPIA_UI,home):MODULES*=homeui

include(qtopiamail.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

