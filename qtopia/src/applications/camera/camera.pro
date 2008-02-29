CONFIG		+= qtopiaapp
HEADERS		= mainwindow.h \
		    thumbbutton.h \
		    videocaptureview.h
SOURCES		= mainwindow.cpp\
		    videocaptureview.cpp \
		    main.cpp

TARGET		= camera

TRANSLATABLES   =   $$HEADERS \
                    $$SOURCES \
                    camerabase.ui \
                    camerasettings.ui \


i18n.path=$${INSTALL_PREFIX}/i18n
i18n.commands=$${COMMAND_HEADER}\
    TRANSLATIONS="$$TRANSLATIONS"; for lang in \$$TRANSLATIONS; do \
	for pkg in Categories-camera; do \
	    $${DQTDIR}/bin/lrelease $${QTOPIA_DEPOT_PATH}/i18n/\$$lang/\$$pkg.ts \
		-qm $(INSTALL_ROOT)/i18n/\$$lang/\$$pkg.qm; \
	done; \
    done
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/camera.desktop
desktop.path=/apps/Applications

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=camera*

pics.files=$${QTOPIA_DEPOT_PATH}/pics/camera/*
pics.path=/pics/camera
service.files=$${QTOPIA_DEPOT_PATH}/services/GetValue/image/camera
service.path=/services/GetValue/image
INSTALLS+=desktop service
HELP_INSTALLS+=help
PICS_INSTALLS+=pics
!isEmpty(DQTDIR):INSTALLS+=i18n

INTERFACES=camerabase.ui camerasettings.ui

PACKAGE_DESCRIPTION=A bogo-camera to fill a hole in the phone launcher.
