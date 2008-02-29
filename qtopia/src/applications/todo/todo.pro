HEADERS+=mainwindow.h
SOURCES+=mainwindow.cpp main.cpp

include (todo.pri)

CONFIG+=pimlib


service.files=$${QTOPIA_DEPOT_PATH}/services/Tasks/todolist
service.path=/services/Tasks
recservice.files=$${QTOPIA_DEPOT_PATH}/services/Receive/text/x-vCalendar-Tasks/todolist
recservice.path=/services/Receive/text/x-vCalendar-Tasks
qdlservice.files=$${QTOPIA_DEPOT_PATH}/services/qdl/todolist
qdlservice.path=/services/qdl
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/todolist.desktop
desktop.path=/apps/Applications
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=todo*
pics.files=$${QTOPIA_DEPOT_PATH}/pics/todolist/*
pics.path=/pics/todolist
INSTALLS+=service recservice qdlservice desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_NAME=qpe-todo
PACKAGE_DESCRIPTION=Todo helps you manage your list of tasks.
PACKAGE_DEPENDS+=qpe-libqtopiapim1
