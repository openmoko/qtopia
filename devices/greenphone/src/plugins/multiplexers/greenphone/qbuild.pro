requires(enable_cell)
TEMPLATE=plugin
CONFIG+=qtopia
TARGET=greenphonemultiplex

QTOPIA*=comm

PLUGIN_FOR=qtopia
PLUGIN_TYPE=multiplexers

include(greenphone.pro)

