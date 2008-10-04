TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=wavplay

PLUGIN_FOR=qtopia
PLUGIN_TYPE=codecs

QTOPIA*=media
contains(PROJECTS,3rdparty/libraries/gsm):MODULES*=gsm

include(wavplay.pro)
