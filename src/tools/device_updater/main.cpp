/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "packagescanner.h"
#include "localsocketlistener.h"
#include "deviceupdaterdialog.h"
#include "deviceupdater.h"

#include <QApplication>
#include <QTimer>

#include <qdebug.h>

const char *callName = "device_updater";

static void usage( QString msg )
{
    if ( !msg.isEmpty() )
        qWarning() << "Error:" << msg;
    qWarning() << "Usage:" << callName << "[--list|-l] [--nogui|-n] [package_name]";
    qWarning() << "        simple Qtopia device update tool";
    qWarning() << "  -l|--list - list all packages found";
    qWarning() << "  -n|--nogui - no GUI, just command line";
}

int main(int argc, char **argv)
{
    int result = 1;
    callName = argv[0];

    PackageScanner *ps = new PackageScanner(".");
    if ( argc > 1 && !ps->findPackageByName( argv[1] ))
        usage( QString( "Package %1 not found" ).arg( argv[1] ));

    LocalSocketListener *lls = new LocalSocketListener();
    if ( !lls->listen() ) // app is already running, just signal that instance
    {
        LocalSocket ls;
        if ( argc > 1 && ps->findPackageByName( argv[1] ))
            ls.sendRequest( QString( "SendPackage %1" ).arg( argv[1] ));
        else
            ls.sendRequest( "RaiseWindow" );
        result = 0;
    }
    else
    {
        QApplication app(argc, argv);
        lls->setupNotifier();
        DeviceUpdaterDialog *du = new DeviceUpdaterDialog();
        du->updater()->connectScanner( ps );
        du->updater()->connectLocalSocket( lls );
        if ( argc > 1 && ps->findPackageByName( argv[1] ))
            du->updater()->sendPackage( argv[1] );
        du->show();
        result = app.exec();
        delete du;
    }
    delete ps;
    delete lls;
    return result;
}
