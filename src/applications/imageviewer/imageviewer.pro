CONFIG          += qtopiaapp

HEADERS		= showimg.h \
		  settingsdialog.h

SOURCES		= settingsdialog.cpp \
		  showimg.cpp \
		  main.cpp

INTERFACES	= settingsdialogbase.ui

TARGET          = imageviewer

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES \
                    $$INTERFACES

help.files=\
    $${QTOPIA_DEPOT_PATH}/help/html/showimg*\
    $${QTOPIA_DEPOT_PATH}/help/html/imageviewer*
help.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/imageviewer.desktop
desktop.path=/apps/Applications
pic.files=$${QTOPIA_DEPOT_PATH}/pics/slideshow.png
pic.path=/pics
INSTALLS+=desktop help
PICS_INSTALLS+=pics

PACKAGE_NAME=qpe-showimg
PACKAGE_DESCRIPTION=The image viewer for the Qtopia environment.
