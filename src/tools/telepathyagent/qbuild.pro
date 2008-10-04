requires(enable_dbus)
TEMPLATE=app
TARGET=telepathyagent

CONFIG+=qtopia singleexec
QTOPIA+=phone collective pim
QT+=dbus

include(telepathyagent.pro)

