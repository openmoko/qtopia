/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifdef QTOPIA_APP_INTERFACE
#undef QTOPIA_APP_INTERFACE
#endif

#ifdef REAL_FILE
#include REAL_FILE
#else

#ifdef LINK_QTOPIA

#include <QWidget>
#include <qtopiaapplication.h>

class LTMain : public QWidget
{
public:
    LTMain( QWidget *parent = 0, Qt::WFlags flags = 0 )
        : QWidget( parent, flags )
    {
    }
};
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, LTMain)
QTOPIA_MAIN

#elif defined(LINK_QT)

#include <QWidget>
#include <QApplication>

class LTMain : public QWidget
{
public:
    LTMain( QWidget *parent = 0, Qt::WFlags flags = 0 )
        : QWidget( parent, flags )
    {
    }
};

int main( int argc, char **argv )
{
    QApplication app(argc, argv);
    LTMain w;
    w.show();
    return app.exec();
}

#else

int main( int argc, char **argv )
{
    return 0;
}

#endif

#endif


