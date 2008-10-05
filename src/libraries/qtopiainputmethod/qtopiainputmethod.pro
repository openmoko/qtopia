qtopia_project(qtopia lib)
TARGET=qtopiainputmethod

HEADERS += qwsinputmethod_x11.h
SOURCES += qwsinputmethod_x11.cpp
LIBS += -lXtst

TRANSLATABLES*=$$HEADERS $$SOURCES

sdk_qtopiainputmethod_headers.files=$${HEADERS}
sdk_qtopiainputmethod_headers.path=/include/qtopia/inputmethod
sdk_qtopiainputmethod_headers.hint=sdk headers
INSTALLS+=sdk_qtopiainputmethod_headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)

