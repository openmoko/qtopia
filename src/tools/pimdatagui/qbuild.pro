TEMPLATE=app
CONFIG+=qtopia singleexec
TARGET=pimdatagui

CONFIG+=quicklaunch

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

include(pimdatagui.pro)

