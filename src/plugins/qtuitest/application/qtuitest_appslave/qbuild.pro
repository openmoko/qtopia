TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=qtuitest_appslave

PLUGIN_TYPE=qtuitest_application
PLUGIN_FOR=qtopia

MODULES*=qtuitest
MODULE_NAME=qtuitest_appslave

SOURCEPATH+=.. \
    ../../slave_qt  \
    /src/libraries/qtuitest

include(../application.pro)
