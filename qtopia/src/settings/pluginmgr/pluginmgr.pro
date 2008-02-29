CONFIG		+= qtopiaapp

HEADERS		= pluginmanager.h
SOURCES		= pluginmanager.cpp main.cpp
INTERFACES	= pluginmanagerbase.ui
TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= pluginmanager

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=pluginmanager.html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/pluginmanager/*
pics.path=/pics/pluginmanager/
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/PluginManager.desktop
desktop.path=/apps/Settings
INSTALLS += desktop
PICS_INSTALLS+=pics
