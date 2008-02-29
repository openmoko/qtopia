qtopia_project(qtopia app)
TARGET=photoedit
# This can't be turned on while photoedit checks qApp->arguments in the photoeditUi constructor!
#CONFIG+=qtopia_main
# Have replaced it for now -- this hasn't solved the problem, and I've been having probs with
# qotpia/photoedit getting mixed up, since, at runtime.
CONFIG+=qtopia_main

HEADERS    = photoeditui.h \
                selector/selectorui.h \
                editor/editorui.h \
                editor/imageui.h \
                editor/slider.h \
                editor/navigator.h \
                editor/regionselector.h \
                editor/matrix.h \
                editor/imageprocessor.h \
                editor/imageio.h \
                slideshow/slideshowui.h \
                slideshow/slideshowdialog.h \
                slideshow/slideshow.h

SOURCES    = main.cpp \
                photoeditui.cpp \
                selector/selectorui.cpp \
                editor/editorui.cpp \
                editor/imageui.cpp \
                editor/slider.cpp \
                editor/navigator.cpp \
                editor/regionselector.cpp \
                editor/imageprocessor.cpp \
                editor/imageio.cpp \
                slideshow/slideshowui.cpp \
                slideshow/slideshowdialog.cpp \
                slideshow/slideshow.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=photoedit*
help.hint=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/photoedit.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/photoedit/*
pics.path=/pics/photoedit
pics.hint=pics
service.files=$$QTOPIA_DEPOT_PATH/services/PhotoEdit/photoedit
service.path=/services/PhotoEdit
qdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/PhotoEdit
qdsservice.path=/etc/qds
INSTALLS+=help desktop pics service qdsservice

pkg.desc=An image manager for Qtopia.
pkg.domain=window,qds,beaming,cardreader,drm,docapi{image/*},lightandpower,pictures,print
