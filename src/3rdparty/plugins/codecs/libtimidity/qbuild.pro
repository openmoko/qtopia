TEMPLATE=plugin
TARGET=timidityplugin

PLUGIN_FOR=qtopia
PLUGIN_TYPE=codecs

CONFIG+=qtopia
QTOPIA+=media
MODULES*=libtimidity

include(libtimidity.pro)

