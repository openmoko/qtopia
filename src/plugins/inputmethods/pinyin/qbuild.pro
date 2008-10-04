TEMPLATE=plugin
TARGET=pinyinim

PLUGIN_FOR=qtopia
PLUGIN_TYPE=inputmethods

CONFIG+=qtopia singleexec
MODULES*=inputmatch

include(pinyin.pro)
