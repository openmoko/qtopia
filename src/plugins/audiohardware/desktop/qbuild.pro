TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=desktopaudiohardware

PLUGIN_FOR=qtopia
PLUGIN_TYPE=audiohardware

QTOPIA*=audio comm

include(desktop.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

