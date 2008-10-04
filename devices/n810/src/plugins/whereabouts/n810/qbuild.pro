requires(enable_qtopiawhereabouts)
TEMPLATE=plugin
TARGET=n810gpsplugin

PLUGIN_FOR=qtopia
PLUGIN_TYPE=whereabouts

CONFIG+=qtopia
QTOPIA+=whereabouts

include(n810.pro)
