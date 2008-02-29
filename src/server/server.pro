qtopia_project(qtopia app)
TARGET=qpe

CONFIG+=enable_phone_ui
# CONFIG += enable_tests

# DEFINES += QTOPIA_PHONEUI

qtopia_performance_test {
    SERVER_HEADERS+=performancetest.h
    SERVER_SOURCES+=performancetest.cpp
}

SERVER_FORMS+=\
    ui/shutdown.ui

SERVER_HEADERS+=\
    qabstractserverinterface.h \
    windowmanagement.h \
    inputmethods.h \
    systemsuspend.h \
    ui/shutdownimpl.h \
    inputdevicesettings.h \
    launcherview.h\
    qcoprouter.h\
    dbusrouter.h\
    alertservicetask.h\
    qtopiapowermanager.h\
    powermanagertask.h\
    qtopiapowermanagerservice.h\
    virtualkeyboardservice.h \
    lowmemorytask.h \
    qabstractdevicemanager.h\
    applicationmonitor.h\
    ui/desktoppoweralerter.h \
    storagemonitor.h \
    qtopiaserverapplication.h \
    environmentsetuptask.h \
    memorymonitor.h \
    ui/criticalmemorypopup.h \
    ui/standarddialogs.h \
    testmemorymonitor.h \
    devicebuttontask.h \
    pressholdgate.h \
    genericmemorymonitor.h \
    keyclick.h \
    applicationlauncher.h \
    qterminationhandlerprovider.h \
    qcopfile.h \
    networkserver.h \
    startupapps.h \
    timemonitor.h \
    qdeviceindicatorsprovider.h \
    shutdownsplash.h \
    screenclick.h \
    vpnmanager.h \
    qkeyboardlock.h \
    standarddevicefeatures.h \
    contentsetlauncherview.h \
    stabmonitortask.h \
    defaultsignal.h \
    dbmigratetask.h

SERVER_SOURCES+=\
    main.cpp \
    qabstractserverinterface.cpp \
    windowmanagement.cpp \
    systemsuspend.cpp \
    systemsuspendtasks.cpp \
    inputmethods.cpp \
    ui/shutdownimpl.cpp \
    inputdevicesettings.cpp \
    launcherview.cpp\
    qcoprouter.cpp\
    dbusrouter.cpp\
    alertservicetask.cpp \
    qtopiapowermanager.cpp\
    powermanagertask.cpp\
    qtopiapowermanagerservice.cpp \
    virtualkeyboardservice.cpp \
    lowmemorytask.cpp \
    qabstractdevicemanager.cpp \
    applicationmonitor.cpp \
    ui/desktoppoweralerter.cpp \
    storagemonitor.cpp \
    qtopiaserverapplication.cpp \
    environmentsetuptask.cpp \
    memorymonitor.cpp \
    ui/criticalmemorypopup.cpp \
    ui/standarddialogs.cpp \
    testmemorymonitor.cpp \
    devicebuttontask.cpp \
    pressholdgate.cpp \
    genericmemorymonitor.cpp \
    keyclick.cpp \
    applicationlauncher.cpp \
    qterminationhandlerprovider.cpp \
    qcopfile.cpp \
    screenclick.cpp \
    networkserver.cpp \
    shutdownsplash.cpp \
    startupapps.cpp \
    timemonitor.cpp \
    qdeviceindicatorsprovider.cpp \
    simplebuiltins.cpp \
    qkeyboardlock.cpp \
    vpnmanager.cpp \
    standarddevicefeatures.cpp \
    contentsetlauncherview.cpp \
    dbmigratetask.cpp \
    stabmonitortask.cpp \
    applicationshutdowntask.cpp \
    defaultsignal.cpp

build_helix {
    DEFINES+=MEDIA_SERVER_PATH=$${LITERAL_ESCAPED_QUOTE}bin/mediaserver$$LITERAL_ESCAPED_QUOTE
}

!build_helix {
    DEFINES+=MEDIA_SERVER_PATH=$${LITERAL_ESCAPED_QUOTE}bin/qss$$LITERAL_ESCAPED_QUOTE
}

SERVER_HEADERS+=obexservicemanager.h
SERVER_SOURCES+=obexservicemanager.cpp

equals(LAUNCH_METHOD,quicklaunch) {
    SERVER_SOURCES+=quickexeapplicationlauncher.cpp
    SERVER_HEADERS+=quickexeapplicationlauncher.h
}

SXE_HEADERS=qsxemangle.h securitymonitor.h sandboxedprocess.h
SXE_SOURCES=qsxemangle.cpp securitymonitor.cpp sandboxedprocess.cpp

TRANSLATABLES+=$$SXE_HEADERS $$SXE_SOURCES

phone:enable_cell:enable_sxe {
    SERVER_HEADERS+=$$SXE_HEADERS
    SERVER_SOURCES+=$$SXE_SOURCES
}

DOCAPI_HEADERS=contentserver.h
DOCAPI_SOURCES=contentserver.cpp
TRANSLATABLES+=$$DOCAPI_HEADERS $$DOCAPI_SOURCES

SERVER_HEADERS+=$$DOCAPI_HEADERS
SERVER_SOURCES+=$$DOCAPI_SOURCES

VPATH+=$$QTOPIA_DEPOT_PATH/src/settings/language
LANGUAGE_FORMS=languagesettingsbase.ui
LANGUAGE_HEADERS=langmodel.h languagesettings.h
LANGUAGE_SOURCES=langmodel.cpp language.cpp
TRANSLATABLES+=$$LANGUAGE_FORMS $$LANGUAGE_HEADERS $$LANGUAGE_SOURCES

VPATH+=$$QTOPIA_DEPOT_PATH/src/settings/systemtime
SYSTEMTIME_HEADERS=settime.h
SYSTEMTIME_SOURCES=settime.cpp
TRANSLATABES+=$$SYSTEMTIME_HEADERS $$SYSTEMTIME_SOURCES

!phone {
    !enable_singleexec|!contains(PROJECTS,settings/language) {
        SERVER_FORMS+=$$LANGUAGE_FORMS
        SERVER_HEADERS+=$$LANGUAGE_HEADERS
        SERVER_SOURCES+=$$LANGUAGE_SOURCES
    }
    !enable_singleexec|!contains(PROJECTS,settings/systemtime) {
        SERVER_HEADERS+=$$SYSTEMTIME_HEADERS
        SERVER_SOURCES+=$$SYSTEMTIME_SOURCES
    }
}

VPATH+=$$QTOPIA_DEPOT_PATH/src/settings/calibrate
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/settings/calibrate
CALIBRATE_HEADERS=calibrate.h
CALIBRATE_SOURCES=calibrate.cpp

HEADERS+=$$CALIBRATE_HEADERS
SOURCES+=$$CALIBRATE_SOURCES

BLUETOOTH_HEADERS=audiomanager.h \
                  bluetooth/bluetoothservicemanager.h \
		  bluetooth/btpinhelper.h \
                  bluetooth/btpowerservice.h \
                  bluetooth/hs/btheadsettask.h \
                  bluetooth/hs/qbluetoothhsagserver_p.h \
                  bluetooth/hs/qbluetoothhsservice_p.h \
                  bluetooth/scomisc_p.h

BLUETOOTH_SOURCES=audiomanager.cpp \
                  bluetooth/bluetoothservicemanager.cpp \
                  bluetooth/btpinhelper.cpp \
                  bluetooth/btpowerservice.cpp \
                  bluetooth/hs/btheadsettask.cpp \
                  bluetooth/hs/qbluetoothhsagserver.cpp \
                  bluetooth/hs/qbluetoothhsservice.cpp

isEmpty(DEVICE_CONFIG_PATH)|!exists($$DEVICE_CONFIG_PATH/server/scomisc.cpp) {
    BLUETOOTH_SOURCES+=bluetooth/scomisc.cpp
}

BLUETOOTH_PHONE_HEADERS+=bluetooth/dun/btdialupservice.h \
                  bluetooth/hf/bthandsfreetask.h \
                  bluetooth/hf/qbluetoothhfagserver_p.h \
                  bluetooth/hf/qbluetoothhfservice_p.h

BLUETOOTH_PHONE_SOURCES+=bluetooth/dun/btdialupservice.cpp \
                  bluetooth/hf/bthandsfreetask.cpp \
                  bluetooth/hf/qbluetoothhfagserver.cpp \
                  bluetooth/hf/qbluetoothhfservice.cpp

TRANSLATABLES+=$$BLUETOOTH_HEADERS $$BLUETOOTH_SOURCES $$BLUETOOTH_PHONE_HEADERS $$BLUETOOTH_PHONE_SOURCES

INFRARED_HEADERS=irpowerservice.h

INFRARED_SOURCES=irpowerservice.cpp

TRANSLATABLES+=$$INFRARED_HEADERS $$INFRARED_SOURCES

PDA_HEADERS=\
    pda/launcher.h\
    pda/appicons.h\
    pda/launchertab.h\
    pda/wait.h \
    pda/taskbar.h\
    pda/runningappbar.h\
    pda/systray.h\
    pda/startmenu.h\
    pda/firstuse.h
PDA_SOURCES=\
    pda/launcher.cpp\
    pda/appicons.cpp\
    pda/launchertab.cpp\
    pda/wait.cpp \
    pda/taskbar.cpp\
    pda/runningappbar.cpp\
    pda/systray.cpp\
    pda/startmenu.cpp\
    pda/firstuse.cpp
TRANSLATABLES=$$PDA_HEADERS $$PDA_SOURCES

# Platform Edition uses the PDA files
pda {
    HEADERS+=$$PDA_HEADERS
    SOURCES+=$$PDA_SOURCES
}

#begin media
MEDIA_HEADERS=\
    media/mediaserver.h \
    media/audiovolumemanager.h \
    media/mediakeyservice.h \
    media/mediaservicestask.h

MEDIA_SOURCES=\
    media/mediaserver.cpp \
    media/audiovolumemanager.cpp \
    media/mediakeyservice.cpp \
    media/mediaservicestask.cpp  

HEADERS+=$$MEDIA_HEADERS
SOURCES+=$$MEDIA_SOURCES
#end media

PHONE_HEADERS=\
    phone/contextlabel.h \
    phone/themecontrol.h \
    phone/documentview.h \
    phone/phonebrowser.h \
    phone/cameramonitor.h \
    phone/alarmcontrol.h \
    phone/phonethemeview.h \
    phone/homescreen.h \
    phone/qphoneprofileprovider.h \
    phone/messagebox.h \
    phone/phoneheader.h\
    phone/phonelock.h\
    phone/taskmanagerservice.h\
    phone/qabstractmessagebox.h\
    phone/qabstractbrowserscreen.h\
    phone/phonelauncher.h \
    phone/qabstractsecondarydisplay.h \
    phone/qsoftmenubarprovider.h \
    phone/secondarythemeddisplay.h \
    phone/receivewindow.h \
    phone/homescreenwidgets.h

TELEPHONY_HEADERS=\
    phone/externalaccess.h \
    phone/messagecontrol.h \
    phone/callhistory.h \
    phone/callcontactlist.h \
    phone/callscreen.h \
    phone/dialercontrol.h \
    phone/savetocontacts.h \
    phone/quickdial.h \
    phone/numberdisplay.h \
    phone/dialer.h \
    phone/dialerservice.h \
    phone/qabstractdialerscreen.h\
    phone/phoneserver.h \
    phone/ringcontrol.h\
    phone/servercontactmodel.h

PHONE_SOURCES=\
    phone/contextlabel.cpp \
    phone/phonelauncher.cpp \
    phone/themecontrol.cpp \
    phone/documentview.cpp \
    phone/phonebrowser.cpp \
    phone/cameramonitor.cpp \
    phone/alarmcontrol.cpp \
    phone/phonethemeview.cpp \
    phone/homescreen.cpp \
    phone/qphoneprofileprovider.cpp \
    phone/messagebox.cpp \
    phone/phoneheader.cpp\
    phone/phonelock.cpp\
    phone/qabstractmessagebox.cpp\
    phone/taskmanagerservice.cpp\
    phone/qabstractsecondarydisplay.cpp \
    phone/qsoftmenubarprovider.cpp \
    phone/secondarythemeddisplay.cpp \
    phone/phonepowermanager.cpp \
    phone/receivewindow.cpp \
    phone/homescreenwidgets.cpp

TELEPHONY_SOURCES=\
    phone/externalaccess.cpp \
    phone/messagecontrol.cpp \
    phone/callhistory.cpp \
    phone/callcontactlist.cpp\
    phone/callscreen.cpp \
    phone/dialercontrol.cpp \
    phone/savetocontacts.cpp \
    phone/quickdial.cpp \
    phone/numberdisplay.cpp \
    phone/dialer.cpp \
    phone/dialerservice.cpp \
    phone/ringcontrol.cpp\
    phone/phoneserver.cpp \
    phone/servercontactmodel.cpp

enable_modem {
    PHONE_HEADERS+=phone/cellmodemmanager.h\
                   phone/cellbroadcastcontrol.h\
                   phone/phoneserverdummymodem.h\
                   phone/gsmkeyactions.h\
                   phone/gsmkeyfilter.h
    PHONE_SOURCES+=phone/cellmodemmanager.cpp\
                   phone/cellbroadcastcontrol.cpp\
                   phone/phoneserverdummymodem.cpp\
                   phone/gsmkeyactions.cpp\
                   phone/gsmkeyfilter.cpp
}

enable_voip {
    PHONE_HEADERS+=\
        phone/phoneservervoipsocket.h\
        phone/voipmanager.h
    PHONE_SOURCES+=\
        phone/phoneservervoipsocket.cpp\
        phone/voipmanager.cpp
}

PHONE_UI_SOURCES=\
    phone/ui/phonelauncherview.cpp \
    phone/ui/griditem.cpp \
    phone/ui/selecteditem.cpp \
    phone/ui/selecteditemconnector.cpp \
    phone/ui/griditemtable.cpp \
    phone/ui/oscillator_p.cpp \
    phone/ui/parabola_p.cpp \
    phone/ui/animator_p.cpp \
    phone/ui/animatorfactory_p.cpp \
    phone/ui/bouncer_p.cpp \
    phone/ui/zoomer_p.cpp \
    phone/ui/shearer_p.cpp \
    phone/ui/rotator_p.cpp \
    phone/ui/radialbackground_p.cpp


PHONE_UI_HEADERS=\
    phone/ui/phonelauncherview.h \
    phone/ui/griditem.h \
    phone/ui/selecteditem.h \
    phone/ui/selecteditemconnector.h \
    phone/ui/griditemtable.h \
    phone/ui/oscillator_p.h \
    phone/ui/parabola_p.h \
    phone/ui/animator_p.h \
    phone/ui/animatorfactory_p.cpp \
    phone/ui/bouncer_p.h \
    phone/ui/zoomer_p.h \
    phone/ui/shearer_p.h \
    phone/ui/rotator_p.h \
    phone/ui/radialbackground_p.h

enable_phone_ui {
    HEADERS+=$$PHONE_UI_HEADERS
    SOURCES+=$$PHONE_UI_SOURCES
}

SAMPLES_SOURCES=\
    phone/samples/slideinmessagebox.cpp \
    phone/samples/wheelbrowser.cpp \
    phone/samples/qpixmapwheel.cpp

SAMPLES_HEADERS=\
    phone/samples/slideinmessagebox.h \
    phone/samples/wheelbrowser.h \
    phone/samples/qpixmapwheel.h

enable_modem:enable_cell {
    SAMPLES_SOURCES+=\
        phone/samples/e1/e1_bar.cpp\
        phone/samples/e1/e1_battery.cpp\
        phone/samples/e1/e1_header.cpp\
        phone/samples/e1/e1_launcher.cpp\
        phone/samples/e1/e1_phonebrowser.cpp\
        phone/samples/e1/colortint.cpp\
        phone/samples/e1/e1_dialer.cpp\
        phone/samples/e1/e1_error.cpp\
        phone/samples/e1/e1_incoming.cpp\
        phone/samples/e1/e1_dialog.cpp\
        phone/samples/e1/e1_callhistory.cpp\
        phone/samples/e1/e1_popup.cpp\
        phone/samples/e1/e1_callscreen.cpp\
        phone/samples/e1/e1_telephony.cpp\
        phone/samples/e2/e2_header.cpp\
        phone/samples/e2/e2_launcher.cpp\
        phone/samples/e2/e2_frames.cpp\
        phone/samples/e2/e2_bar.cpp\
        phone/samples/e2/e2_taskmanager.cpp\
        phone/samples/e2/e2_telephony.cpp\
        phone/samples/e2/e2_callscreen.cpp\
        phone/samples/e2/e2_dialer.cpp\
        phone/samples/e2/e2_browser.cpp\
        phone/samples/e2/e2_telephonybar.cpp

    SAMPLES_HEADERS+=\
        phone/samples/e1/e1_bar.h\
        phone/samples/e1/e1_battery.h\
        phone/samples/e1/e1_header.h\
        phone/samples/e1/e1_launcher.h\
        phone/samples/e1/e1_dialog.h\
        phone/samples/e1/e1_dialer.h\
        phone/samples/e1/e1_error.h\
        phone/samples/e1/e1_callhistory.h\
        phone/samples/e1/e1_phonebrowser.h\
        phone/samples/e1/e1_popup.h\
        phone/samples/e1/e1_callscreen.h\
        phone/samples/e1/colortint.h\
        phone/samples/e1/e1_incoming.h\
        phone/samples/e1/e1_telephony.h\
        phone/samples/e2/e2_header.h\
        phone/samples/e2/e2_launcher.h\
        phone/samples/e2/e2_frames.h\
        phone/samples/e2/e2_bar.h\
        phone/samples/e2/e2_telephony.h\
        phone/samples/e2/e2_callscreen.h\
        phone/samples/e2/e2_dialer.h\
        phone/samples/e2/e2_browser.h\
        phone/samples/e2/e2_taskmanager.h\
        phone/samples/e2/e2_telephonybar.h\
        phone/samples/e2/e2_colors.h
}

samplespics.files=$$QTOPIA_DEPOT_PATH/pics/samples/*
samplespics.path=/pics/samples
samplespics.hint=pics

samplesprofilepics.files=$$QTOPIA_DEPOT_PATH/pics/profiles/*
samplesprofilepics.path=/pics/profiles
samplesprofilepics.hint=pics

enable_samples {
    HEADERS+=$$SAMPLES_HEADERS
    SOURCES+=$$SAMPLES_SOURCES
    INSTALLS+=samplespics samplesprofilepics
}

TESTS_SOURCES += test/cellmodemmanagertest.cpp
TESTS_HEADERS += test/cellmodemmanagertest.h

enable_tests {
    HEADERS+=$$TESTS_HEADERS
    SOURCES+=$$TESTS_SOURCES
}

TRANSLATABLES+=$$PHONE_HEADERS $$PHONE_SOURCES

phone {
    enable_bluetooth {
        PHONE_HEADERS+=$$BLUETOOTH_HEADERS
        PHONE_SOURCES+=$$BLUETOOTH_SOURCES
    }
    enable_infrared {
        PHONE_HEADERS+=$$INFRARED_HEADERS
        PHONE_SOURCES+=$$INFRARED_SOURCES
    }

    # This is necessary for Handsfree / Headset to work
    equals(QTOPIA_SOUND_SYSTEM,alsa) {
    	depends(3rdparty/libraries/alsa)
    	DEFINES+=HAVE_ALSA
    }

    HEADERS+=$$PHONE_HEADERS
    SOURCES+=$$PHONE_SOURCES
    # phone ui
    !free_package|free_plus_binaries {
        HEADERS+=$$TELEPHONY_HEADERS
        DEFINES+=QTOPIA_PHONEUI
        SOURCES+=$$TELEPHONY_SOURCES
        enable_bluetooth {
            HEADERS+=$$BLUETOOTH_PHONE_HEADERS
            SOURCES+=$$BLUETOOTH_PHONE_SOURCES
        }
        depends(libraries/qtopiaphone)
    }
    depends(libraries/qtopiapim)
    !enable_qtopiabase:depends(libraries/qtopiail)
    depends(3rdparty/libraries/openobex)
    depends(libraries/qtopiacomm)

    enable_modem {
        depends(libraries/qtopiaphonemodem)
    }
}

qtopiatest {
    depends(libraries/qtopiatest/qtesttools/target)
    depends(libraries/qtopiatest/qsystemtestslave)
    depends(libraries/qtopiatest/qtopiasystemtestslave)
    depends(libraries/qtopiatest/qtopiaservertestslave)
}

!isEmpty(DEVICE_CONFIG_PATH) {
    SERVER_HEADERS+=$$files($$DEVICE_CONFIG_PATH/server/*.h)
    SERVER_SOURCES+=$$files($$DEVICE_CONFIG_PATH/server/*.cpp)
}

enable_sxe:depends(libraries/qtopiasecurity)

drmagent {
    SERVER_HEADERS+=archiveviewer.h
    SERVER_SOURCES+=archiveviewer.cpp
    archivesdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/archives.desktop
    archivesdesktop.path=/apps/Applications
    archivesdesktop.hint=desktop
    archiveshelp.source=$$QTOPIA_DEPOT_PATH/help
    archiveshelp.files=qpe-archives*
    archiveshelp.hint=help
    INSTALLS+=archivesdesktop archiveshelp
}

FORMS+=$$SERVER_FORMS
HEADERS+=$$SERVER_HEADERS
SOURCES+=$$SERVER_SOURCES

sdk_server_headers.files=$$SERVER_HEADERS
sdk_server_headers.path=/src/server
sdk_server_headers.hint=sdk
INSTALLS+=sdk_server_headers

sdk_calibrate_headers.files=$$CALIBRATE_HEADERS
sdk_calibrate_headers.path=/src/settings/calibrate
sdk_calibrate_headers.hint=sdk
INSTALLS+=sdk_calibrate_headers

phone {
    sdk_edn_headers.files=$$PHONE_HEADERS
    sdk_edn_headers.path=/src/server/phone
} else {
    sdk_edn_headers.files=$$PDA_HEADERS
    sdk_edn_headers.path=/src/server/pda
}
sdk_edn_headers.hint=sdk
INSTALLS+=sdk_edn_headers

bins.files=\
    $$QTOPIA_DEPOT_PATH/bin/targzip\
    $$QTOPIA_DEPOT_PATH/bin/targunzip
bins.path=/bin
bins.hint=script
INSTALLS+=bins

taskmanagerdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/TaskManager.desktop
taskmanagerdesktop.path=/apps/Settings
taskmanagerdesktop.hint=desktop
INSTALLS+=taskmanagerdesktop

calibratedesktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/Calibrate.desktop
calibratedesktop.path=/apps/Settings
calibratedesktop.hint=desktop
INSTALLS+=calibratedesktop

calibrateservice.files=$$QTOPIA_DEPOT_PATH/services/calibrate/calibrate
calibrateservice.path=/services/calibrate
INSTALLS+=calibrateservice

launcherservice.files=$$QTOPIA_DEPOT_PATH/services/Launcher/qpe
launcherservice.path=/services/Launcher
INSTALLS+=launcherservice

contentsetviewservice.files=$$QTOPIA_DEPOT_PATH/services/ContentSetView/qpe
contentsetviewservice.path=/services/ContentSetView
INSTALLS+=contentsetviewservice

alertservice.files=$$QTOPIA_DEPOT_PATH/services/Alert/qpe
alertservice.path=/services/Alert
INSTALLS+=alertservice

qtopiapowermanager.files=$$QTOPIA_DEPOT_PATH/services/QtopiaPowerManager/qpe
qtopiapowermanager.path=/services/QtopiaPowerManager
INSTALLS+=qtopiapowermanager

virtualkeyboardservice.files=$$QTOPIA_DEPOT_PATH/services/VirtualKeyboard/qpe
virtualkeyboardservice.path=/services/VirtualKeyboard
INSTALLS+=virtualkeyboardservice

touchscreendialerservice.files=$$QTOPIA_DEPOT_PATH/services/TouchscreenDialer/qpe
touchscreendialerservice.path=/services/TouchscreenDialer
INSTALLS+=touchscreendialerservice

calibratepics.files=$$QTOPIA_DEPOT_PATH/pics/calibrate/*
calibratepics.path=/pics/calibrate
calibratepics.hint=pics
INSTALLS+=calibratepics

appspics.files=$$QTOPIA_DEPOT_PATH/pics/*.* $$QTOPIA_DEPOT_PATH/pics/icons
appspics.path=/pics
appspics.hint=pics
INSTALLS+=appspics

pics.files=$$QTOPIA_DEPOT_PATH/pics/qpe/*.* $$QTOPIA_DEPOT_PATH/pics/qpe/icons
pics.path=/pics/qpe
pics.hint=pics
INSTALLS+=pics

wallpaperpics.files=$$QTOPIA_DEPOT_PATH/pics/wallpaper/*.png
wallpaperpics.path=/pics/wallpaper
wallpaperpics.hint=content nct
wallpaperpics.categories=SystemWallpapers
wallpaperpics.trtarget=QtopiaWallpapers
INSTALLS+=wallpaperpics

settings.files=\
    $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/locale.conf\
    $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/FontMap.conf\
    $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/ServerWidgets.conf\
    $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/Security.conf\
    $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/IniValueSpace.conf\
    $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/BluetoothServices.conf\
    $$QPEDIR/etc/default/Trolltech/IconSizes.conf
!isEmpty(HARDWARE_CONF_FILE) {
    settings.files+=$$HARDWARE_CONF_FILE
}
settings.path=/etc/default/Trolltech
INSTALLS+=settings

# qpe.conf gets modified for Free builds
qpe_conf.path=/etc/default/Trolltech
qpe_conf.commands=$$COMMAND_HEADER\
    rm -f $(INSTALL_ROOT)$$qpe_conf.path/qpe.conf $$LINE_SEP\
    cat $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/qpe.conf
free_package:!free_plus_binaries {
    # Free build (ie. non-phone) defaults to smart theme
    qpe_conf.commands+=\
        | sed 's/qtopia\.conf/smart\.conf/'
}
qpe_conf.commands+=\
    > $(INSTALL_ROOT)$$qpe_conf.path/qpe.conf
INSTALLS+=qpe_conf

!enable_singleexec {
    inputmethods.files=$$QTOPIA_DEPOT_PATH/plugins/inputmethods/.directory
    inputmethods.path=/plugins/inputmethods
    INSTALLS+=inputmethods

    obex.files=$$QTOPIA_DEPOT_PATH/plugins/obex/.directory
    obex.path=/plugins/obex
    INSTALLS+=obex

    network.files=$$QTOPIA_DEPOT_PATH/plugins/network/.directory
    network.path=/plugins/network
    INSTALLS+=network
}

speeddialsounds.files=$$QTOPIA_DEPOT_PATH/sounds/speeddial
speeddialsounds.path=/sounds
INSTALLS+=speeddialsounds

servicedefs.files=$$files($$QTOPIA_DEPOT_PATH/services/*.service)
!enable_cell {
    servicedefs.files -= $$QTOPIA_DEPOT_PATH/services/SMS.service
    servicedefs.files -= $$QTOPIA_DEPOT_PATH/services/CallForwarding.service
    servicedefs.files -= $$QTOPIA_DEPOT_PATH/services/CallNetworks.service
    servicedefs.files -= $$QTOPIA_DEPOT_PATH/services/Dialer.service
}
!drmagent {
    servicedefs.files -= $$QTOPIA_DEPOT_PATH/services/OmaDrmAgent.service
}
servicedefs.path=/services
INSTALLS+=servicedefs

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=\
    index.html\
    qpe*\
    help-*\
    help.html\
    document*\
    device*\
    appservices.html\
    calibrat*
help.hint=help
INSTALLS+=help

beam.files=$$QTOPIA_DEPOT_PATH/etc/beam
beam.path=/etc
INSTALLS+=beam

!isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/Tasks.cfg) {
    tasks.files=$$DEVICE_CONFIG_PATH/etc/Tasks.cfg
} else {
    tasks.files=$$QTOPIA_DEPOT_PATH/etc/Tasks.cfg
}
tasks.path=/etc
INSTALLS+=tasks

defaultbuttons {
    defbtn.files=$$QPEDIR/etc/defaultbuttons.conf
    defbtn.path=/etc
    INSTALLS+=defbtn
}

suspendservice.files=$$QTOPIA_DEPOT_PATH/services/Suspend/qpe
suspendservice.path=/services/Suspend
INSTALLS+=suspendservice

enable_infrared {
    irbeamingservice.files=$$QTOPIA_DEPOT_PATH/services/InfraredBeaming/qpe
    irbeamingservice.path=/services/InfraredBeaming
    INSTALLS+=irbeamingservice

    irqdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/InfraredBeaming
    irqdsservice.path=/etc/qds
    INSTALLS+=irqdsservice
}

enable_bluetooth {
    btpushservice.files=$$QTOPIA_DEPOT_PATH/services/BluetoothPush/qpe
    btpushservice.path=/services/BluetoothPush
    INSTALLS+=btpushservice

    btqdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/BluetoothPush
    btqdsservice.path=/etc/qds
    INSTALLS+=btqdsservice

    btservices.files=$$QTOPIA_DEPOT_PATH/etc/bluetooth/sdp/*.xml
    btservices.path=/etc/bluetooth/sdp
    INSTALLS+=btservices
}

phone {
    phonepics.files=$$QTOPIA_DEPOT_PATH/pics/qpe/phone/*
    phonepics.path=/pics/qpe/phone
    phonepics.hint=pics
    INSTALLS+=phonepics

    globalphonepics.files=$$QTOPIA_DEPOT_PATH/pics/phone/*
    globalphonepics.path=/pics/phone
    globalphonepics.hint=pics
    INSTALLS+=globalphonepics

    settings.files+=$$QTOPIA_DEPOT_PATH/etc/default/Trolltech/PhoneProfile.conf

    defaultalerts.files=$$QTOPIA_DEPOT_PATH/etc/SystemRingTones/*.wav
    defaultalerts.path=/etc/SystemRingTones
    defaultalerts.hint=content nct
    defaultalerts.categories=SystemRingTones
    defaultalerts.trtarget=QtopiaRingTones
    INSTALLS+=defaultalerts

    !free_package|free_plus_binaries {
        callhistorydesktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/callhistory.desktop
        callhistorydesktop.path=/apps/Applications
        callhistorydesktop.hint=desktop
        INSTALLS+=callhistorydesktop

        callhistorypics.files=$$QTOPIA_DEPOT_PATH/pics/callhistory/*
        callhistorypics.path=/pics/callhistory
        callhistorypics.hint=pics
        INSTALLS+=callhistorypics

        callhistoryservice.files=$$QTOPIA_DEPOT_PATH/services/callhistory/qpe
        callhistoryservice.path=/services/callhistory
        INSTALLS+=callhistoryservice
    }

    taskmanagerservice.files=$$QTOPIA_DEPOT_PATH/services/TaskManager/qpe
    taskmanagerservice.path=/services/TaskManager
    INSTALLS+=taskmanagerservice

    defaulttheme.files=$$QTOPIA_DEPOT_PATH/etc/themes/default/*
    defaulttheme.path=/etc/themes/default
    INSTALLS+=defaulttheme

    defaultpics.files=$$QTOPIA_DEPOT_PATH/pics/themes/default*
    defaultpics.path=/pics/themes
    defaultpics.hint=pics
    INSTALLS+=defaultpics

    !free_package|free_plus_binaries {
        dialerservice.files=$$QTOPIA_DEPOT_PATH/services/Dialer/qpe
        dialerservice.path=/services/Dialer
        INSTALLS+=dialerservice
    }

    ANIMFILE=$$QTOPIA_DEPOT_PATH/pics/qpe/splash/$${QTOPIA_DISP_WIDTH}x$${QTOPIA_DISP_HEIGHT}/splash.gif
    exists($$ANIMFILE) {
        splash.files=$$ANIMFILE
    } else {
        # Fall-back un-animated
        splash.files=$$QTOPIA_DEPOT_PATH/src/server/splash.png
    }
    splash.path=/pics/qpe
    INSTALLS+=splash
} else {
    shutdown.files=$$QTOPIA_DEPOT_PATH/services/shutdown/shutdown
    shutdown.path=/services/shutdown
    INSTALLS+=shutdown

    pdapics.files=$$QTOPIA_DEPOT_PATH/pics/qpe/pda/*
    pdapics.path=/pics/qpe/pda
    pdapics.hint=pics
    INSTALLS+=pdapics

    !enable_singleexec {
        applets.files=$$QTOPIA_DEPOT_PATH/plugins/applets/.directory
        applets.path=/plugins/applets
        INSTALLS+=applets
    }

    help.files+=\
	categor*\
	about*\
	backuprestore.html\
	beaming.html\
	sync.html\
	popup-calendar.html
}

enable_sxe {
    security.path=/etc
    security.commands=$$COMMAND_HEADER\
        install -c $$QTOPIA_DEPOT_PATH/etc/sxe.* $(INSTALL_ROOT)/etc $$LINE_SEP\
        chmod 0600 $(INSTALL_ROOT)/etc/sxe.* $$LINE_SEP\
        mkdir -p $(INSTALL_ROOT)/etc/rekey $$LINE_SEP\
        mkdir -p $(INSTALL_ROOT)/etc/sxe_qtopia $$LINE_SEP\
        mkdir -p $(INSTALL_ROOT)/etc/sxe_domains $$LINE_SEP

    SXE_SCRIPTS=sxe_qtopia sxe_sandbox sxe_unsandbox
    for(file,SXE_SCRIPTS) {
        !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/sxe_qtopia/$$file) {
            security.commands+=\
                install -m 0500 -c $$DEVICE_CONFIG_PATH/etc/sxe_qtopia/$$file $(INSTALL_ROOT)/etc/sxe_qtopia $$LINE_SEP
        } else {
            security.commands+=\
                install -m 0500 -c $$QTOPIA_DEPOT_PATH/etc/sxe_qtopia/$$file $(INSTALL_ROOT)/etc/sxe_qtopia $$LINE_SEP
        }
    }

    security.commands+=\
        install -m 0500 -c $$QTOPIA_DEPOT_PATH/etc/sxe_domains/* $(INSTALL_ROOT)/etc/sxe_domains $$LINE_SEP

    !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/sxe_domains) {
        security.commands+=\
            install -m 0500 -c $$DEVICE_CONFIG_PATH/etc/sxe_domains/* $(INSTALL_ROOT)/etc/sxe_domains $$LINE_SEP
    }
            
    !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/default/Trolltech/SxeMonitor.conf) {
        settings.files+=$$DEVICE_CONFIG_PATH/etc/default/Trolltech/SxeMonitor.conf
    } else {
        settings.files+=$$QTOPIA_DEPOT_PATH/etc/default/Trolltech/SxeMonitor.conf
    }

    INSTALLS+=security
}

# We want an en_US .directory file and dictionaries, even if en_US was disabled via -languages
commands=$$COMMAND_HEADER
INST_LANGUAGES=$$LANGUAGES
INST_LANGUAGES*=en_US
for(lang,INST_LANGUAGES) {
    !equals(commands,$$COMMAND_HEADER):commands+=$$LINE_SEP
    commands+=\
        mkdir -p $(INSTALL_ROOT)$$resdir/i18n/$$lang $$LINE_SEP_VERBOSE\
        install -c $$QTOPIA_DEPOT_PATH/i18n/$$lang/.directory $(INSTALL_ROOT)$$resdir/i18n/$$lang
}
langfiles.commands=$$commands
langfiles.CONFIG=no_path
INSTALLS+=langfiles

commands=$$COMMAND_HEADER
for(lang,INST_LANGUAGES) {
    !equals(commands,$$COMMAND_HEADER):commands+=$$LINE_SEP
    exists($$QTOPIA_DEPOT_PATH/etc/dict/$$lang/words):commands+=\
        mkdir -p $(INSTALL_ROOT)$$resdir/etc/dict/$$lang $$LINE_SEP\
        find $$QTOPIA_DEPOT_PATH/etc/dict/$$lang -maxdepth 1 -type f | xargs -r $$QPEDIR/bin/qdawggen $(INSTALL_ROOT)$$resdir/etc/dict/$$lang
}
dicts.commands=$$commands
dicts.CONFIG=no_path
INSTALLS+=dicts

EXTRA_TS_FILES=\
    QtopiaApplications QtopiaGames QtopiaSettings QtopiaI18N QtopiaServices\
    QtopiaNetworkServices QtopiaBeaming QtopiaColorSchemes QtopiaDefaults\
    QtopiaPlugins QtopiaRingTones \
    QtopiaThemes Categories-Qtopia

# lupdate for "global" stuff
nct_lupdate.commands=$$COMMAND_HEADER\
    cd $$PWD;\
    $$QPEDIR/bin/nct_lupdate\
        -nowarn\
        -depot\
        $$LITERAL_QUOTE$$QTOPIA_DEPOT_PATH$$LITERAL_QUOTE\
        $$LITERAL_QUOTE$$DQTDIR$$LITERAL_QUOTE\
        $$LITERAL_QUOTE$$TRANSLATIONS$$LITERAL_QUOTE\
	`find $$QTOPIA_DEPOT_PATH\
	    # apps, plugins and i18n
	    $${LITERAL_BACKSLASH}( $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/apps/*$${LITERAL_SQUOTE} -o\
	          -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/plugins/*$${LITERAL_SQUOTE} -o\
	          -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/i18n/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) -a\
	       # .directory and .desktop files
	       $${LITERAL_BACKSLASH}( -name $${LITERAL_SQUOTE}.directory$${LITERAL_SQUOTE} -o -name $${LITERAL_SQUOTE}*.desktop$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) $${LITERAL_BACKSLASH}) -o\
	    # etc
	    $${LITERAL_BACKSLASH}( $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/etc/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) -a\
	       # .conf and .scheme files
	       $${LITERAL_BACKSLASH}( -name $${LITERAL_SQUOTE}*.conf$${LITERAL_SQUOTE} -o -name $${LITERAL_SQUOTE}*.scheme$${LITERAL_SQUOTE} -o -name $${LITERAL_SQUOTE}*.desktop$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) $${LITERAL_BACKSLASH}) -o\
            # qds
	    $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/etc/qds/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) -o\
	    # pics
	    $${LITERAL_BACKSLASH}( $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/pics/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) -a\
	       # config files (media player skins)
	       $${LITERAL_BACKSLASH}( -name $${LITERAL_SQUOTE}config$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) $${LITERAL_BACKSLASH}) -o\
	    # services (all files)
	    $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/services/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH})`
lupdate.depends+=nct_lupdate
QMAKE_EXTRA_TARGETS+=nct_lupdate

pkg.name=qpe-taskbar
pkg.desc=Launcher for QPE
pkg.domain=base,launcher,policy,syslog,prefix

# FIXME THIS SHOULD NOT BE HERE!!!
dep(INCLUDEPATH+=$$PWD)
dep(pkg.deps+=$$pkg.name)

enable_singleexec {
    # Depend on everything so we can guarantee that this directory is processed last
    for(p,PROJECTS) {
        !equals(p,server):!contains(QTOPIA_DEPENDS,$$p):depends($$p,fake)
    }

    # Final preparation for singleexec build
    # This is hidden from other .pro files because it is very expensive
    equals(QTOPIA_ID,server) {
        # The server can't be built as a .a file
        CONFIG-=enable_singleexec
        # However, we still need this define so singleexec can be detected in code
        DEFINES+=SINGLE_EXEC
        # This is rediculous. Just ignore it.
        CONFIG-=link_prl

        cmds=$$fromfile(singleexec_reader.pri,SINGLEEXEC_READER_CMD)
        #message(singleexec_reader tells us:)
        for(c,cmds) {
            contains(QMAKE_BEHAVIORS,keep_quotes) {
                c~=s/^"//
                c~=s/"$//
            }
            #message($$c)
            runlast($$c)
        }
        # This avoids problems due to incorrect ordering of libs.
        # I think it tells the linker to treat all these seperate files as one giant library.
        # Surely this is slow and ineffienent but it does seem to work :)
        cmd=LIBS=-Wl,-whole-archive \$$LIBS -Wl,-no-whole-archive
        runlast($$cmd)
        #message(Done! $$LIBS)
    }

    # qmake isn't putting in these dependencies so do it outselves
    files=$$files($$QPEDIR/lib/*.a)
    files*=$$files($$QTEDIR/lib/*.a)
    qpe.depends=$$files
    qpe.commands=
    QMAKE_EXTRA_TARGETS+=qpe

    ipatchqt.commands=$$COMMAND_HEADER\
        $$fixpath($$QPEDIR/bin/patchqt) $$fixpath($(INSTALL_ROOT)/bin/qpe) $$QTOPIA_PREFIX
    ipatchqt.CONFIG=no_path
    ipatchqt.depends=install_target
    INSTALLS+=ipatchqt
}

