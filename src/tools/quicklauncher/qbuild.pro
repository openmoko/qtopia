#requires(!equals(LAUNCH_METHOD,normal))
TEMPLATE=app
CONFIG+=qtopia singleexec
TARGET=quicklauncher

contains(QTOPIA_MODULES,pim):QTOPIA*=pim
QTOPIA.CONFIG=singleexec

include(quicklauncher.pro)

