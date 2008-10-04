TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=qtuitestwidgets

PLUGIN_TYPE=qtuitest_widgets
PLUGIN_FOR=qtopia

MODULES*=qtuitest
QTOPIA+=theming

SOURCEPATH+=../widgets_qt

include(widgets_qtopia.pro)
