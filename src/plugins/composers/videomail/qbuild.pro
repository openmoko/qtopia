TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=videomailcomposer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=composers

QTOPIA*=mail media
MODULES*=homeui

include(videomail.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

