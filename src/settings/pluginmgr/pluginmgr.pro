CONFIG		+= qtopiaapp

HEADERS		= pluginmanager.h
SOURCES		= pluginmanager.cpp main.cpp
INTERFACES	= pluginmanagerbase.ui
TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= pluginmanager

help.files=$${QTOPIA_DEPOT_PATH}/help/html/pluginmanager.html
help.path=/help/html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/pluginmanager/*
pics.path=/pics/pluginmanager/
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/PluginManager.desktop
desktop.path=/apps/Settings
INSTALLS += desktop
PICS_INSTALLS+=pics
