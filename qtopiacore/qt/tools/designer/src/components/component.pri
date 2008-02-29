
win32|mac:CONFIG += debug_and_release
contains(TEMPLATE, ".*lib") {
   CONFIG(debug, debug|release) {
      mac:TARGET = $$member(TARGET, 0)_debug
      win32:TARGET = $$member(TARGET, 0)d
   }
}


