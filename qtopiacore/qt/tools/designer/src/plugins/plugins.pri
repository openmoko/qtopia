CONFIG += designer
win32|mac: CONFIG+= debug_and_release
DESTDIR = $$QT_BUILD_TREE/plugins/designer
contains(TEMPLATE, ".*lib") {
   CONFIG(debug, debug|release) {
      mac:TARGET = $$member(TARGET, 0)_debug
      win32:TARGET = $$member(TARGET, 0)d
   }
}

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
