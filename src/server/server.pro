qtopia_project(qtopia app)
TARGET=qpe

CONFIG+=enable_phone_ui

!enable_singleexec {
    # This allows plugins to access symbols in the server.
    # qtopiatest uses this.
    QMAKE_LFLAGS*=-Wl,--export-dynamic
}
enable_singleexec:qtopiatest {
    CONFIG += qtestlib
}
# CONFIG += enable_tests

# DEFINES += QTOPIA_PHONEUI

SERVER_FORMS+=\
    ui/shutdown.ui \
    ui/volume.ui

SERVER_HEADERS+=\
    qabstractserverinterface.h \
    systemsuspend.h \
    ui/shutdownimpl.h \
    ui/volumeimpl.h \
    launcherview.h\
    qcoprouter.h\
    dbusrouter.h\
    alertservicetask.h\
    qabstractdevicemanager.h\
    applicationmonitor.h\
    storagemonitor.h \
    qtopiaserverapplication.h \
    environmentsetuptask.h \
    memorymonitor.h \
    ui/standarddialogs.h \
    ui/delayedwaitdialog.h \
    testmemorymonitor.h \
    pressholdgate.h \
    genericmemorymonitor.h \
    applicationlauncher.h \
    oommanager.h \
    qterminationhandlerprovider.h \
    qcopfile.h \
    networkserver.h \
    startupapps.h \
    timemonitor.h \
    timeupdateservice.h \
    qdeviceindicatorsprovider.h \
    shutdownsplash.h \
    contentsetlauncherview.h \
    stabmonitortask.h \
    defaultsignal.h \
    dbmigratetask.h \
    defaultbattery.h\
    apmbattery.h\
    obexservicemanager.h \
    keyclick.h \
    devicebuttontask.h \
    inputmethods.h \
    inputdevicesettings.h \
    lowmemorytask.h \
    windowmanagement.h \
    virtualkeyboardservice.h \
    qtopiapowermanager.h\
    qtopiapowermanagerservice.h \
    standarddevicefeatures.h \
    qtopiainputevents.h \
    waitindicator.h

SERVER_SOURCES+=\
    main.cpp \
    qabstractserverinterface.cpp \
    systemsuspend.cpp \
    systemsuspendtasks.cpp \
    ui/shutdownimpl.cpp \
    ui/volumeimpl.cpp \
    launcherview.cpp\
    qcoprouter.cpp\
    dbusrouter.cpp\
    alertservicetask.cpp \
    qabstractdevicemanager.cpp \
    applicationmonitor.cpp \
    storagemonitor.cpp \
    qtopiaserverapplication.cpp \
    environmentsetuptask.cpp \
    memorymonitor.cpp \
    ui/standarddialogs.cpp \
    ui/delayedwaitdialog.cpp \
    testmemorymonitor.cpp \
    pressholdgate.cpp \
    genericmemorymonitor.cpp \
    applicationlauncher.cpp \
    oommanager.cpp \
    qterminationhandlerprovider.cpp \
    qcopfile.cpp \
    networkserver.cpp \
    shutdownsplash.cpp \
    startupapps.cpp \
    timemonitor.cpp \
    timeupdateservice.cpp \
    qdeviceindicatorsprovider.cpp \
    simplebuiltins.cpp \
    contentsetlauncherview.cpp \
    defaultbattery.cpp\
    apmbattery.cpp\
    qdsynctask.cpp\
    obexservicemanager.cpp\
    dbmigratetask.cpp \
    stabmonitortask.cpp \
    applicationshutdowntask.cpp \
    defaultsignal.cpp \
    keyclick.cpp \
    devicebuttontask.cpp \
    inputmethods.cpp \
    inputdevicesettings.cpp \
    lowmemorytask.cpp \
    virtualkeyboardservice.cpp \
    qtopiapowermanager.cpp\
    qtopiapowermanagerservice.cpp \
    standarddevicefeatures.cpp \
    waitindicator.cpp

enable_vpn {
    SERVER_HEADERS+=vpnmanager.h
    SERVER_SOURCES+=vpnmanager.cpp
}

!x11 {
    SERVER_HEADERS+=\
        screenclick.h \
        qkeyboardlock.h
    SERVER_SOURCES+=\
        screenclick.cpp \
        qkeyboardlock.cpp \
        windowmanagement.cpp \
        qtopiainputevents.cpp
} else {
    SERVER_SOURCES+=\
        windowmanagement_x11.cpp \
        qtopiainputevents_x11.cpp

    LIBS += -lXtst
}

equals(LAUNCH_METHOD,quicklaunch) {
    SERVER_SOURCES+=quickexeapplicationlauncher.cpp
    SERVER_HEADERS+=quickexeapplicationlauncher.h
}

SXE_HEADERS=securitymonitor.h 
SXE_SOURCES=securitymonitor.cpp 
enable_cell {
    SXE_HEADERS+=qsxemangle.h
    SXE_SOURCES+=qsxemangle.cpp
}

TRANSLATABLES+=$$SXE_HEADERS $$SXE_SOURCES

enable_sxe {
    SERVER_HEADERS+=$$SXE_HEADERS
    SERVER_SOURCES+=$$SXE_SOURCES
}

DOCAPI_HEADERS=contentserver.h
DOCAPI_SOURCES=contentserver.cpp
TRANSLATABLES+=$$DOCAPI_HEADERS $$DOCAPI_SOURCES

SERVER_HEADERS+=$$DOCAPI_HEADERS
SERVER_SOURCES+=$$DOCAPI_SOURCES

VPATH+=$$QTOPIA_DEPOT_PATH/src/settings/calibrate
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/settings/calibrate
CALIBRATE_HEADERS=calibrate.h
CALIBRATE_SOURCES=calibrate.cpp

!x11 {
    HEADERS+=$$CALIBRATE_HEADERS
    SOURCES+=$$CALIBRATE_SOURCES
}

VPATH+=$$QTOPIA_DEPOT_PATH/src/applications/simapp
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/applications/simapp
SIMAPP_HEADERS=simapp.h simicons.h simwidgets.h
SIMAPP_SOURCES=simapp.cpp simicons.cpp simwidgets.cpp

enable_cell {
    HEADERS+=$$SIMAPP_HEADERS
    SOURCES+=$$SIMAPP_SOURCES
}

BLUETOOTH_HEADERS=bluetooth/bluetoothservicemanager.h \
		  bluetooth/btpinhelper.h \
                  bluetooth/btpowerservice.h \
                  bluetooth/hs/btheadsettask.h \
                  bluetooth/hs/qbluetoothhsagserver_p.h \
                  bluetooth/hs/qbluetoothhsservice_p.h \
                  bluetooth/scomisc_p.h \
                  bluetooth/btaudiovolumemanager_p.h \
                  bluetooth/ftp/btftpservice.h

BLUETOOTH_SOURCES=bluetooth/bluetoothservicemanager.cpp \
                  bluetooth/btpinhelper.cpp \
                  bluetooth/btpowerservice.cpp \
                  bluetooth/hs/btheadsettask.cpp \
                  bluetooth/hs/qbluetoothhsagserver.cpp \
                  bluetooth/hs/qbluetoothhsservice.cpp \
                  bluetooth/btaudiovolumemanager.cpp \
                  bluetooth/ftp/btftpservice.cpp

# This is documented in src/build/doc/src/deviceprofiles.qdoc
isEmpty(DEVICE_CONFIG_PATH)|!exists($$DEVICE_CONFIG_PATH/server/scomisc.cpp) {
    BLUETOOTH_SOURCES+=bluetooth/scomisc.cpp
}

BLUETOOTH_PHONE_HEADERS+=bluetooth/dun/btdialupservice.h \
                  bluetooth/bluetoothserialportservice.h \
                  bluetooth/hf/bthandsfreetask.h \
                  bluetooth/hf/qbluetoothhfagserver_p.h \
                  bluetooth/hf/qbluetoothhfservice_p.h

BLUETOOTH_PHONE_SOURCES+=bluetooth/dun/btdialupservice.cpp \
                         bluetooth/bluetoothserialportservice.cpp \
                  bluetooth/hf/bthandsfreetask.cpp \
                  bluetooth/hf/qbluetoothhfagserver.cpp \
                  bluetooth/hf/qbluetoothhfservice.cpp

TRANSLATABLES+=$$BLUETOOTH_HEADERS $$BLUETOOTH_SOURCES $$BLUETOOTH_PHONE_HEADERS $$BLUETOOTH_PHONE_SOURCES

INFRARED_HEADERS=irpowerservice.h

INFRARED_SOURCES=irpowerservice.cpp

TRANSLATABLES+=$$INFRARED_HEADERS $$INFRARED_SOURCES

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

UNPORTED_HEADERS=firstuse.h
UNPORTED_SOURCES=firstuse.cpp

PHONE_HEADERS=\
    phone/contextlabel.h \
    phone/themecontrol.h \
    phone/documentview.h \
    phone/phonebrowser.h \
    phone/runningapplicationsviewitem.h \
    phone/cameramonitor.h \
    phone/alarmcontrol.h \
    phone/phonethemeview.h \
    phone/homescreencontrol.h \
    phone/qabstracthomescreen.h \
    phone/themedhomescreen.h \
    phone/themebackground_p.h \
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
    phone/homescreenwidgets.h \
    phone/phonepowermanager.h \
    phone/gprsmonitor.h \
    phone/qabstractthemewidgetfactory.h \
    phone/touchscreenlockdlg.h

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
    phone/ringtoneservice.h\
    phone/servercontactmodel.h \
    phone/qabstractcallpolicymanager.h \
    phone/videoringtone.h

PHONE_SOURCES=\
    phone/contextlabel.cpp \
    phone/phonelauncher.cpp \
    phone/themecontrol.cpp \
    phone/documentview.cpp \
    phone/phonebrowser.cpp \
    phone/runningapplicationsviewitem.cpp \
    phone/cameramonitor.cpp \
    phone/alarmcontrol.cpp \
    phone/phonethemeview.cpp \
    phone/qabstracthomescreen.cpp \
    phone/themedhomescreen.cpp \
    phone/themebackground_p.cpp \
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
    phone/homescreenwidgets.cpp \
    phone/gprsmonitor.cpp \
    phone/touchscreenlockdlg.cpp

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
    phone/ringtoneservice.cpp\
    phone/phoneserver.cpp \
    phone/servercontactmodel.cpp \
    phone/qabstractcallpolicymanager.cpp \
    phone/videoringtone.cpp

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
        phone/voipmanager.h\
        phone/asteriskmanager.h
    PHONE_SOURCES+=\
        phone/phoneservervoipsocket.cpp\
        phone/voipmanager.cpp\
        phone/asteriskmanager.cpp
}

enable_hierarchicaldocumentview {
    PHONE_HEADERS+=\
        phone/hierarchicaldocumentview.h
    PHONE_SOURCES+=\
        phone/hierarchicaldocumentview.cpp
        
    DEFINES+=ENABLE_HIERARCHICAL_DOCUMENT_VIEW
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
    phone/ui/radialbackground_p.cpp \
    phone/ui/renderer.cpp


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
    phone/ui/radialbackground_p.h \
    phone/ui/renderer.h

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
        phone/samples/e2/e2_telephonybar.cpp\
        phone/samples/e3/e3_phonebrowser.cpp\
        phone/samples/e3/e3_launcher.cpp\
        phone/samples/e3/e3_today.cpp\
        phone/samples/e3/e3_navipane.cpp\
        phone/samples/e3/e3_clock.cpp

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
        phone/samples/e2/e2_colors.h\
        phone/samples/e3/e3_phonebrowser.h\
        phone/samples/e3/e3_launcher.h\
        phone/samples/e3/e3_today.h\
        phone/samples/e3/e3_navipane.h\
        phone/samples/e3/e3_clock.h

samples_settings.files=\
    $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/E3.conf
samples_settings.path=/etc/default/Trolltech
INSTALLS+=samples_settings
}

enable_samples {
    HEADERS+=$$SAMPLES_HEADERS
    SOURCES+=$$SAMPLES_SOURCES

    samplespics.files=$$QTOPIA_DEPOT_PATH/pics/samples/*
    samplespics.path=/pics/samples
    samplespics.hint=pics
    INSTALLS+=samplespics

    samplesprofilepics.files=$$QTOPIA_DEPOT_PATH/pics/profiles/*
    samplesprofilepics.path=/pics/profiles
    samplesprofilepics.hint=pics
    INSTALLS+=samplesprofilepics
}

TESTS_SOURCES += test/cellmodemmanagertest.cpp
TESTS_HEADERS += test/cellmodemmanagertest.h

enable_tests {
    HEADERS+=$$TESTS_HEADERS
    SOURCES+=$$TESTS_SOURCES
}

TRANSLATABLES+=$$PHONE_HEADERS $$PHONE_SOURCES

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

isEmpty(DEVICE_CONFIG_PATH) {
    PHONE_HEADERS+=phone/dummyvolumeservice.h
    PHONE_SOURCES+=phone/dummyvolumeservice.cpp
}

HEADERS+=$$PHONE_HEADERS
SOURCES+=$$PHONE_SOURCES

# phone ui
HEADERS+=$$TELEPHONY_HEADERS
DEFINES+=QTOPIA_PHONEUI
SOURCES+=$$TELEPHONY_SOURCES
enable_bluetooth {
    HEADERS+=$$BLUETOOTH_PHONE_HEADERS
    SOURCES+=$$BLUETOOTH_PHONE_SOURCES
}
depends(libraries/qtopiaphone)
depends(libraries/qtopiapim)
depends(3rdparty/libraries/openobex)
depends(libraries/qtopiacomm)
enable_qtopiamedia {
    depends(libraries/qtopiamedia)
}
depends(libraries/qtopiaaudio)

enable_modem {
    depends(libraries/qtopiaphonemodem)
}

# This is documented in src/build/doc/src/deviceprofiles.qdoc
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
    INSTALLS+=archivesdesktop
    archiveshelp.source=$$QTOPIA_DEPOT_PATH/help
    archiveshelp.files=qpe-archives*
    archiveshelp.hint=help
    INSTALLS+=archiveshelp
    archivespics.files=$$QTOPIA_DEPOT_PATH/pics/archives/*
    archivespics.path=/pics/archives
    archivespics.hint=pics
    INSTALLS+=archivespics
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

sdk_edn_headers.files=$$PHONE_HEADERS
sdk_edn_headers.path=/src/server/phone
sdk_edn_headers.hint=sdk
INSTALLS+=sdk_edn_headers

bins.files=\
    $$QTOPIA_DEPOT_PATH/bin/targzip\
    $$QTOPIA_DEPOT_PATH/bin/targunzip
bins.path=/bin
bins.hint=script
INSTALLS+=bins

taskmanagerdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/taskmanager.desktop
taskmanagerdesktop.path=/apps/Settings
taskmanagerdesktop.hint=desktop
INSTALLS+=taskmanagerdesktop

calibratedesktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/calibrate.desktop
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

timeupdateservice.files=$$QTOPIA_DEPOT_PATH/services/TimeUpdate/qpe
timeupdateservice.path=/services/TimeUpdate
INSTALLS+=timeupdateservice

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

# Wallpapers no longer used.
#wallpaperpics.files=$$QTOPIA_DEPOT_PATH/pics/wallpaper/*.png
#wallpaperpics.path=/pics/wallpaper
#wallpaperpics.hint=content nct
#wallpaperpics.categories=SystemWallpapers
#wallpaperpics.trtarget=QtopiaWallpapers
#INSTALLS+=wallpaperpics

enable_cell {
    sdk_simapp_headers.files=$$SIMAPP_HEADERS
    sdk_simapp_headers.path=/src/applications/simapp
    sdk_simapp_headers.hint=sdk
    INSTALLS+=sdk_simapp_headers

    simappdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/simapp.desktop
    simappdesktop.path=/apps/Applications
    simappdesktop.hint=desktop
    INSTALLS+=simappdesktop

    simapppics.files=$$QTOPIA_DEPOT_PATH/pics/simapp/*
    simapppics.path=/pics/simapp
    simapppics.hint=pics
    INSTALLS+=simapppics
}

settings.files=\
    $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/locale.conf\
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
!isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/default/Trolltech/qpe.conf):qpe_conf.files=$$DEVICE_CONFIG_PATH/etc/default/Trolltech/qpe.conf
else:qpe_conf.files=$$QTOPIA_DEPOT_PATH/etc/default/Trolltech/qpe.conf
qpe_conf.path=/etc/default/Trolltech
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
    servicedefs.files -= $$QTOPIA_DEPOT_PATH/services/SimApp.service
    servicedefs.files -= $$QTOPIA_DEPOT_PATH/services/Ringtone.service
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
    simapp.html\
    callhistory.html\
    calibrat*
help.hint=help
INSTALLS+=help

beam.files=$$QTOPIA_DEPOT_PATH/etc/beam
beam.path=/etc
INSTALLS+=beam

# This is documented in src/build/doc/src/deviceprofiles.qdoc
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
defaultalerts.categories=SystemRingtones
defaultalerts.trtarget=QtopiaRingTones
INSTALLS+=defaultalerts

callhistorydesktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/callhistory.desktop
callhistorydesktop.path=/apps/Applications
callhistorydesktop.hint=desktop
INSTALLS+=callhistorydesktop

callhistorypics.files=$$QTOPIA_DEPOT_PATH/pics/callhistory/*
callhistorypics.path=/pics/callhistory
callhistorypics.hint=pics
INSTALLS+=callhistorypics

callhistoryservice.files=$$QTOPIA_DEPOT_PATH/services/CallHistory/qpe
callhistoryservice.path=/services/CallHistory
INSTALLS+=callhistoryservice

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

dialerservice.files=$$QTOPIA_DEPOT_PATH/services/Dialer/qpe
dialerservice.path=/services/Dialer
INSTALLS+=dialerservice

ringtoneservice.files=$$QTOPIA_DEPOT_PATH/services/Ringtone/qpe
ringtoneservice.path=/services/Ringtone
INSTALLS+=ringtoneservice

ANIMFILE=$$QTOPIA_DEPOT_PATH/pics/qpe/splash/$${QTOPIA_DISP_WIDTH}x$${QTOPIA_DISP_HEIGHT}/splash.gif
exists($$ANIMFILE) {
    splash.files=$$ANIMFILE
} else {
    # Fall-back un-animated
    splash.files=$$QTOPIA_DEPOT_PATH/src/server/splash.png
}
splash.path=/pics/qpe
INSTALLS+=splash

enable_sxe {
    security.path=/etc
    security.commands=$$COMMAND_HEADER\
        install -c $$QTOPIA_DEPOT_PATH/etc/sxe.* $(INSTALL_ROOT)/etc $$LINE_SEP\
        chmod 0600 $(INSTALL_ROOT)/etc/sxe.* $$LINE_SEP\
        mkdir -p $(INSTALL_ROOT)/etc/rekey $$LINE_SEP\
        mkdir -p $(INSTALL_ROOT)/etc/sxe_qtopia $$LINE_SEP\
        mkdir -p $(INSTALL_ROOT)/etc/sxe_domains $$LINE_SEP

    SXE_SCRIPTS=sxe_qtopia sxe_sandbox sxe_unsandbox sxe_reloadconf
    for(file,SXE_SCRIPTS) {
        # This is documented in src/build/doc/src/deviceprofiles.qdoc
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

    # This is documented in src/build/doc/src/deviceprofiles.qdoc
    !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/sxe_domains) {
        security.commands+=\
            install -m 0500 -c $$DEVICE_CONFIG_PATH/etc/sxe_domains/* $(INSTALL_ROOT)/etc/sxe_domains $$LINE_SEP
    }
            
    # This is documented in src/build/doc/src/deviceprofiles.qdoc
    !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/default/Trolltech/Sxe.conf) {
        settings.files+=$$DEVICE_CONFIG_PATH/etc/default/Trolltech/Sxe.conf
    } else {
        settings.files+=$$QTOPIA_DEPOT_PATH/etc/default/Trolltech/Sxe.conf
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
    $$QBS_BIN/nct_lupdate\
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
pkg.domain=base,launcher,quicklauncher,key_manager,modem,policy,prefix,directvideo,directaudio,nice,kill,tmp_write,datetime,rfcomm,netadmin,clock

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
        $$fixpath($$QBS_BIN/patchqt) $$fixpath($(INSTALL_ROOT)/bin/qpe) $$QTOPIA_PREFIX
    ipatchqt.CONFIG=no_path
    ipatchqt.depends=install_target
    INSTALLS+=ipatchqt
}

