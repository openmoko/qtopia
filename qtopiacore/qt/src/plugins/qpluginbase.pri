TEMPLATE = lib
CONFIG += qt plugin

win32|mac:!win32-msvc:!macx-xcode:CONFIG += debug_and_release
!debug_and_release|build_pass {
   CONFIG(debug, debug|release): {
      mac:TARGET = $$member(TARGET, 0)_debug
      win32:TARGET = $$member(TARGET, 0)d
   }
}

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

include(../qt_targets.pri)
