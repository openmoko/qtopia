requires(enable_ffmpeg)
TEMPLATE=plugin
CONFIG+=qtopia
TARGET=ffmpeg

PLUGIN_FOR=qtopia
PLUGIN_TYPE=mediaengines

QTOPIA*=media audio

include(ffmpeg.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
