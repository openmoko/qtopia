CONFIG		+= qtopia

TARGET		= filebrowser
DESTDIR         = $$(QPEDIR)/bin

HEADERS		= inlineedit.h \
		  filebrowser.h
SOURCES		= filebrowser.cpp \
		  inlineedit.cpp \
		  main.cpp

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES \

target.path=/bin
INSTALLS+=target

help.files=$${QTOPIA_DEPOT_PATH}/help/html/filebrowser*
help.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/filebrowser.desktop
desktop.path=/apps/Applications
pics.files=$${QTOPIA_DEPOT_PATH}/pics/filebrowser/*
pics.path=/pics/filebrowser
INSTALLS+=desktop help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=The filebrowser for the Qtopia environment.
