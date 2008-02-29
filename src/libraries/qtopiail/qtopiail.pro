qtopia_project(embedded lib)
TARGET=qtopiail
CONFIG += no_tr qtopia_visibility

SOURCES += framework/applayer.cpp \
           framework/inilayer.cpp \
           framework/qfilemonitor.cpp \
           framework/qmallocpool.cpp \
           framework/qpacketprotocol.cpp \
           framework/qsystemlock.cpp \
           framework/qvaluespace.cpp \
	   framework/qexpressionevaluator.cpp \
           framework/qfixedpointnumber.cpp

HEADERS += framework/qfilemonitor.h \
           framework/qmallocpool.h \
           framework/qpacketprotocol.h \
           framework/qsystemlock.h \
           framework/qvaluespace.h \
	   framework/qexpressionevaluator.h \
           framework/qfixedpointnumber_p.h \
           framework/qtopiailglobal.h

sdk_headers.files=$${HEADERS}
sdk_headers.path=/include/qtopiail
sdk_headers.hint=sdk headers
INCLUDEPATH += framework $$QTOPIA_DEPOT_PATH/src/3rdparty/libraries/qtopiail
INSTALLS+=sdk_headers
depends(libraries/qtopiabase)
idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
