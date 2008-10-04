TEMPLATE=plugin
TARGET=tremorplugin

PLUGIN_FOR=qtopia
PLUGIN_TYPE=codecs

CONFIG+=qtopia
QTOPIA+=media
MODULES*=tremor

include(tremor.pro)

