CONFIG    += qtopiaapp

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

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES \
                    $$INTERFACES

TARGET     = photoedit

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=photoedit*

desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/photoedit.desktop
desktop.path=/apps/Applications
pics.files=$${QTOPIA_DEPOT_PATH}/pics/photoedit/*
pics.path=/pics/photoedit
service.files=$${QTOPIA_DEPOT_PATH}/services/PhotoEdit/photoedit
service.path=/services/PhotoEdit
qdsservice.files=$${QTOPIA_DEPOT_PATH}/services/GetValue/image/photoedit
qdsservice.path=/services/GetValue/image
INSTALLS+=desktop service qdsservice
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_NAME=qpe-photoedit
PACKAGE_DESCRIPTION=An image manager for the Qtopia Environment.
