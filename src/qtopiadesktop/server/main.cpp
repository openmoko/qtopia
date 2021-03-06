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
#include "qtopiadesktopapplication.h"

#include <qtopiadesktoplog.h>

#include <QStringList>

int main( int argc, char **argv )
{
    QtopiaDesktopApplication instance( argc, argv );
    if ( instance.isRunning() ) {
        qDebug() << "Qtopia Sync Agent is already running";
        if ( instance.sendMessage( instance.arguments().join("!!!") ) ) {
            return 0;
        } else {
            qDebug() << "Coult not communicate with the previous instance";
            return 1;
        }
    }

    instance.initialize();
    int ret = instance.exec();
    return ret;
}

