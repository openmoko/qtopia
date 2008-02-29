/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include <QtopiaApplication>
#include <QWidget>
#include <QPluginManager>

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,qdsync)
#define MAIN_FUNC main_qdsync
#else
#define MAIN_FUNC main
#endif

QSXE_APP_KEY
int MAIN_FUNC( int argc, char **argv )
{
    QSXE_SET_APP_KEY(argv[0]);
    QtopiaApplication app( argc, argv );
    QPluginManager manager( "qdsync" );
    QObject *base = manager.instance("base");
    if ( !base ) {
        qWarning() << "Could not load plugin 'base'";
        return -1;
    }
    QWidget *window = qobject_cast<QWidget*>(base);
    if ( !window ) {
        qWarning() << "Plugin 'base' is not a QWidget";
        return -1;
    }
    app.setMainWidget( window );
    return app.exec();
}

