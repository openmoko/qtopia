qtopia_project(qtopia plugin)
TARGET=device_updater
plugin_type=qdsync
# This doesn't work for non-app plugins
CONFIG+=no_link_test

SOURCES=main.cpp

depends(tools/qdsync/common)

