requires(enable_cell)
TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=mmscomposer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=composers

QTOPIA*=mail

include(mms.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

