TEMPLATE=app
CONFIG+=qtopia
TARGET=todolist

QTOPIA*=pim
CONFIG+=quicklaunch singleexec

HEADERS+=\
    todotable.h\
    todoentryimpl.h\
    todocategoryselector.h\
    mainwindow.h\
    recurrencedetails.h\
    reminderpicker.h\
    listpositionbar.h\
    qdelayedscrollarea.h\
    qtopiatabwidget.h

SOURCES+=\
    todotable.cpp\
    todoentryimpl.cpp\
    mainwindow.cpp\
    main.cpp\
    recurrencedetails.cpp\
    reminderpicker.cpp\
    listpositionbar.cpp\
    qdelayedscrollarea.cpp\
    qtopiatabwidget.cpp


service.files=$$QTOPIA_DEPOT_PATH/services/Tasks/todolist
service.path=/services/Tasks
INSTALLS+=service
qdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/Tasks
qdsservice.path=/etc/qds
INSTALLS+=qdsservice
recservice.files=$$QTOPIA_DEPOT_PATH/services/Receive/text/x-vcalendar-Tasks/todolist
recservice.path=/services/Receive/text/x-vcalendar-Tasks
INSTALLS+=recservice
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/todolist.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=todolist*
help.hint=help
INSTALLS+=help

# pics are installed by libqtopiapim since they're shared

pkg.name=qpe-todo
pkg.desc=Todo helps you manage your list of tasks.
pkg.domain=trusted

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

