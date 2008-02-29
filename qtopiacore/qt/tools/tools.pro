TEMPLATE        = subdirs
no-png {
    message("Tools not available without PNG support")
} else {
    unix:contains(QT_CONFIG, qdbus):SUBDIRS += qdbus
    SUBDIRS		+= assistant/lib \
			assistant \
			porting \
                        qtestlib \
			pixeltool
    contains(QT_EDITION, Console) {
        SUBDIRS += designer/src/uitools     # Linguist depends on this
    } else {
        SUBDIRS += designer
    }
    SUBDIRS     += linguist
    unix:!embedded:contains(QT_CONFIG, qt3support):SUBDIRS += qtconfig
    win32:!contains(QT_EDITION, OpenSource|Console):SUBDIRS += activeqt
}

CONFIG+=ordered
QTDIR_build:REQUIRES = "contains(QT_CONFIG, full-config)"
