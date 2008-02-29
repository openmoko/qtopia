TEMPLATE        = app
QT += xml
CONFIG          += qt warn_on console
CONFIG          -= app_bundle
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}


HEADERS         = ../shared/metatranslator.h \
                  ../shared/translator.h \
                  ../shared/proparser.h \
                  ../shared/profileevaluator.h \
                  ../shared/proparserutils.h \
                  ../shared/simtexth.h \
                  ../shared/xliff.h

SOURCES         = fetchtr.cpp \
                  fetchtrjava.cpp \
                  main.cpp \
                  merge.cpp \
                  numberh.cpp \
                  sametexth.cpp \
                  ../shared/metatranslator.cpp \
                  ../shared/translator.cpp \
                  ../shared/proparser.cpp \
                  ../shared/profileevaluator.cpp \
                  ../shared/simtexth.cpp \
                  ../shared/xliff.cpp

PROPARSERPATH = ../shared
INCLUDEPATH += $$PROPARSERPATH
# Input
HEADERS += $$PROPARSERPATH/proitems.h \
        $$PROPARSERPATH/abstractproitemvisitor.h \
        $$PROPARSERPATH/proreader.h
SOURCES += $$PROPARSERPATH/proitems.cpp \
        $$PROPARSERPATH/proreader.cpp


TARGET          = lupdate
INCLUDEPATH     += ../shared
DESTDIR          = ../../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
