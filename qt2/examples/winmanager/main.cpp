/****************************************************************************
** $Id: qt/examples/winmanager/main.cpp   2.3.12   edited 2005-10-27 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qwidget.h>
#include <qfont.h>
#include "../hello/hello.h"
#include "minimal.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QFont f(app.font());
    f.setPointSize(12);
    app.setFont(f);

    app.qwsSetDecoration( new QWSMinimalDecoration() );

    Hello hello("Hello World");
    hello.setCaption("Minimal window manager");
    hello.setFont( QFont("times",32,QFont::Bold) );
    hello.show();

    app.setMainWidget(&hello);

    return app.exec();
}
