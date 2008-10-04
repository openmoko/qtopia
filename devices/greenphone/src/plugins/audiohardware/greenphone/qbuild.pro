TEMPLATE=plugin
CONFIG+=qtopia
TARGET=greenphoneaudiohardware

QTOPIA*=audio
enable_bluetooth:QTOPIA*=comm

PLUGIN_FOR=qtopia
PLUGIN_TYPE=audiohardware

include(greenphone.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

