TEMPLATE=app
CONFIG+=qtopia
TARGET=qtmail

QTOPIA*=mail pim
enable_telephony:QTOPIA*=phone
CONFIG+=quicklaunch singleexec
equals(QTOPIA_UI,home):MODULES*=homeui

include(qtmail.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

