TEMPLATE=plugin
TARGET=inputmethodsitem

PLUGIN_FOR=qtopia
PLUGIN_TYPE=themeitems

CONFIG+=qtopia singleexec
QTOPIA+=theming

#FIXME this isn't really the way you want to do this...
SOURCEPATH+=/src/server/core_server

include(inputmethodsitem.pro)

