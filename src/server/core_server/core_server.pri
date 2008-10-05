HEADERS+=\
         alarmcontrol.h \
         applicationlauncher.h\
         contentserver.h\
         defaultbattery.h\
         devicebuttontask.h \
         environmentsetuptask.h\
         idletaskstartup.h\
         pressholdgate.h\
         qabstractmessagebox.h\
         qabstractserverinterface.h \
         qcopfile.h \
         qcoprouter.h \
         qdeviceindicatorsprovider.h \
         qtopiainputevents.h \
         qtopiapowermanager.h\
         qtopiapowermanagerservice.h \
         qtopiaserverapplication.h \
         qtopiaservertasks_p.h \
         systemsuspend.h \
         timecontrol.h \
         timemonitor.h \
         timeupdateservice.h \
         uifactory.h \
         virtualkeyboardservice.h \
         windowmanagement.h 

SOURCES+=\
         alarmcontrol.cpp \
         applicationlauncher.cpp\
         contentserver.cpp\
         devicebuttontask.cpp \
         defaultbattery.cpp\
         environmentsetuptask.cpp\
         idletaskstartup.cpp\
         pressholdgate.cpp\
         qabstractmessagebox.cpp\
         qabstractserverinterface.cpp \
         qcopfile.cpp \
         qcoprouter.cpp \
         qdeviceindicatorsprovider.cpp \
         qtopiapowermanager.cpp\
         qtopiapowermanagerservice.cpp \
         qtopiaserverapplication.cpp\
         systemsuspend.cpp \
         timecontrol.cpp \
         timemonitor.cpp \
         timeupdateservice.cpp \
         uifactory.cpp \
         virtualkeyboardservice.cpp 

X11.TYPE=CONDITIONAL_SOURCES
X11.CONDITION=x11
X11.SOURCES=\
    qtopiainputevents_x11.cpp \
    windowmanagement_x11.cpp 
!qbuild:CONDITIONAL_SOURCES(X11)

QWS.TYPE=CONDITIONAL_SOURCES
QWS.CONDITION=qws
QWS.SOURCES=\
    qtopiainputevents.cpp\
    windowmanagement.cpp 
!qbuild:CONDITIONAL_SOURCES(QWS)

