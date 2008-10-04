TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=wavrecord

PLUGIN_FOR=qtopia
PLUGIN_TYPE=codecs

contains(PROJECTS,3rdparty/libraries/gsm):MODULES*=gsm

include(wavrecord.pro)
