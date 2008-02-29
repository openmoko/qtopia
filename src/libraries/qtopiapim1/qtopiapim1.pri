VPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtopiapim1/

#QTOPIAPIM1_HEADERS +=

QTOPIAPIM1_SOURCES += datebookaccess1.cpp \
    event1.cpp

qtopiadesktop {
    HEADERS+=$$QTOPIAPIM1_HEADERS
    SOURCES+=$$QTOPIAPIM1_SOURCES
}

