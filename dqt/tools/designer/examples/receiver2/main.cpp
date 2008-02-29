/****************************************************************************
** $Id: qt/main.cpp   3.3.5   edited Aug 31 12:13 $
**
** Copyright (C) 1992-2005 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "mainform.h"


int main( int argc, char *argv[] ) 
{
    QApplication app( argc, argv );

    MainForm *mainForm = new MainForm;
    app.setMainWidget( mainForm );
    mainForm->show(); 

    return app.exec();
}


