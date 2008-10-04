TEMPLATE=plugin
TARGET=bluetoothprinting

PLUGIN_FOR=qtopia
PLUGIN_TYPE=qtopiaprinting

CONFIG+=qtopia singleexec
QTOPIA+=printing comm

include(bluetooth.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

