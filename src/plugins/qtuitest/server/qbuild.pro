TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=qtuitest_qpeslave

PLUGIN_TYPE=qtuitest_server
PLUGIN_FOR=qtopia

MODULES*=qtuitest qtuitest_appslave::headers
SOURCEPATH+=/src/libraries/qtuitest

include(server.pro)
