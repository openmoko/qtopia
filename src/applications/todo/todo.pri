CONFIG		+= qtopiaapp

VPATH		+= $$QTOPIA_DEPOT_PATH/src/applications/todo/

TARGET		= todolist

HEADERS		+= todotable.h \
		  todoentryimpl.h \
		  nulldb.h

SOURCES		+= todotable.cpp \
		  todoentryimpl.cpp

PHONE_INTERFACES    = taskphonedetail.ui
PDA_INTERFACES      = taskdetail.ui

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES \
                    $$PHONE_INTERFACES \
                    $$PDA_INTERFACES
                    

!qtopiadesktop:QTOPIA_PHONE {
    INTERFACES	= $$PHONE_INTERFACES
} else {
    INTERFACES	= $$PDA_INTERFACES 
}

