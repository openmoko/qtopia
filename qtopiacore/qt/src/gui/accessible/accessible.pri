# Qt accessibility module

contains(QT_CONFIG, accessibility) {
      HEADERS += accessible/qaccessible.h \
		 accessible/qaccessibleobject.h \
		 accessible/qaccessiblewidget.h \
		 accessible/qaccessibleplugin.h
      SOURCES += accessible/qaccessible.cpp \
		 accessible/qaccessibleobject.cpp \
		 accessible/qaccessiblewidget.cpp \
		 accessible/qaccessibleplugin.cpp

      mac {
        SOURCES += accessible/qaccessible_mac.cpp
      } else:win32 { 
        SOURCES += accessible/qaccessible_win.cpp
      } else {
        HEADERS += accessible/qaccessiblebridge.h
        SOURCES += accessible/qaccessible_unix.cpp accessible/qaccessiblebridge.cpp
      }
}
