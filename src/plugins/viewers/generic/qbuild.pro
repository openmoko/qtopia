TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=genericviewer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=viewers

QTOPIA*=mail
equals(QTOPIA_UI,home):MODULES*=homeui

include(generic.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

