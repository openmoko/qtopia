qtopia_project(qtopia app)
TARGET=todolist
CONFIG+=qtopia_main

PHONE_FORMS=\
    taskphonedetail.ui
PDA_FORMS=\
    taskdetail.ui

TRANSLATABLES+=$$PHONE_FORMS $$PDA_FORMS

phone:FORMS+=$$PHONE_FORMS
else:FORMS+=$$PDA_FORMS 

HEADERS+=\
    todotable.h\
    taskdelegate.h\
    todoentryimpl.h\
    todocategoryselector.h\
    mainwindow.h

SOURCES+=\
    todotable.cpp\
    taskdelegate.cpp\
    todoentryimpl.cpp\
    mainwindow.cpp\
    main.cpp

depends(libraries/qtopiapim)

service.files=$$QTOPIA_DEPOT_PATH/services/Tasks/todolist
service.path=/services/Tasks
qdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/Tasks
qdsservice.path=/etc/qds
recservice.files=$$QTOPIA_DEPOT_PATH/services/Receive/text/x-vcalendar-Tasks/todolist
recservice.path=/services/Receive/text/x-vcalendar-Tasks
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/todolist.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=todo*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/todolist/*
pics.path=/pics/todolist
pics.hint=pics
INSTALLS+=service qdsservice desktop help pics recservice

pkg.name=qpe-todo
pkg.desc=Todo helps you manage your list of tasks.
pkg.domain=window,beaming,qdl,qds,docapi,pim,bluetooth
