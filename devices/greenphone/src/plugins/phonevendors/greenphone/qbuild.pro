requires(enable_cell)
TEMPLATE=plugin
CONFIG+=qtopia
TARGET=greenphonevendor

QTOPIA*=phonemodem

PLUGIN_FOR=qtopia
PLUGIN_TYPE=phonevendors

include(greenphone.pro)

