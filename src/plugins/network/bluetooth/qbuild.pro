TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=bluetooth

PLUGIN_FOR=qtopia
PLUGIN_TYPE=network

QTOPIA*=comm

include(bluetooth.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

