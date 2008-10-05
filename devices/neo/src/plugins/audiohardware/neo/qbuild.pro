TEMPLATE=plugin
TARGET=neoaudiohardware

PLUGIN_FOR=qtopia
PLUGIN_TYPE=audiohardware

CONFIG+=qtopia
QTOPIA+=audio
enable_bluetooth:QTOPIA+=comm

include(neo.pro)
