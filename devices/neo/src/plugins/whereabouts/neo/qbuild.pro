requires(enable_qtopiawhereabouts)
TEMPLATE=plugin
TARGET=neogpsplugin

PLUGIN_FOR=qtopia
PLUGIN_TYPE=whereabouts

CONFIG+=qtopia
QTOPIA+=whereabouts

include(neo.pro)
