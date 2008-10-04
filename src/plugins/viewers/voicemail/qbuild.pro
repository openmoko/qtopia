requires(enable_qtopiamedia)
TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=voicemailviewer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=viewers

QTOPIA*=mail pim media
equals(QTOPIA_UI,home):MODULES*=homeui

include(voicemail.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

