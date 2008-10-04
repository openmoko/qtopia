requires(enable_cell)
TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=smilviewer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=viewers

QTOPIA*=mail smil

include(smil.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

