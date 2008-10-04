TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET = gstreamer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=mediaengines

QTOPIA*=media video
MODULES*=gstreamer

include(gstreamer.pro)
