!qbuild{
    qtopia_project(qtopia plugin)
    CONFIG += no_tr no_link_test
    plugin_type = qtuitest_widgets
    TARGET=qtuitestwidgets
    INCLUDEPATH+=$$PWD/../widgets_qt
}

SOURCES += \
        localqtopiawidget.cpp \
        remotewidget.cpp \
        remotewidgetadaptor.cpp \
        testcallmanager.cpp \
        testcontextlabel.cpp \
        testdialer.cpp \
        testiconselector.cpp \
        testnumberdisplay.cpp \
        testoptionsmenu.cpp \
        testphonecalc.cpp \
        testphonelauncherview.cpp \
        testphonequickdialerscreen.cpp \
        testphonetouchdialerscreen.cpp \
        testpkim.cpp \
        testpredictivekeyboard.cpp \
        testqtopiafactory.cpp \
        testsmoothlist.cpp \
        testthemedhomescreen.cpp \
        testthemedview.cpp \
        testthemelistmodel.cpp

HEADERS += \
        localqtopiawidget.h \
        remotewidget.h \
        remotewidgetadaptor.h \
        testcallmanager.h \
        testcontextlabel.h \
        testdialer.h \
        testiconselector.h \
        testnumberdisplay.h \
        testoptionsmenu.h \
        testphonecalc.h \
        testphonelauncherview.h \
        testphonequickdialerscreen.h \
        testphonetouchdialerscreen.h \
        testpkim.h \
        testpredictivekeyboard.h \
        testqtopiafactory.h \
        testsmoothlist.h \
        testthemedhomescreen.h \
        testthemedview.h \
        testthemelistmodel.h

!qbuild{
    depends(libraries/qtuitest)
    depends(libraries/qtopiatheming)
}

