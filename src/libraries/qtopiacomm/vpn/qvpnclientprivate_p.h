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

#include "qvpnclient.h"

#include <QProcess>
#include <QString>
#include <qvaluespace.h>

#include <qtopianamespace.h>

class QVPNClientPrivate {
public:
    QVPNClientPrivate()
        : vpnProcess( 0 ), vSpace( 0 ), vItem( 0 )
    {
        serverMode = false;
    }

    ~QVPNClientPrivate() {
        if ( vpnProcess ) {
            delete vpnProcess;
            vpnProcess = 0;
        }
        if ( vSpace )  {
            delete vSpace;
            vSpace = 0;
        }
        if ( vItem ) {
            delete vItem;
            vItem = 0;
        }
    }

    /*!
      Created by non-server mode object.
      */
    void createVPNConfig(const QString& baseName)
    {
        if ( serverMode )
            return;

        const QString path = Qtopia::applicationFileName( "Network", "vpn" );
        QDir settingsDir(path);
        QString filename;
        int n = 0;
        do {
            filename = settingsDir.filePath( baseName +
                    QString::number(n++)+ QLatin1String(".conf") );
        } while ( QFile::exists( filename ) );

        config = filename;
    }

    QProcess* vpnProcess;
    QString config;
    bool serverMode;
    QString errorString;
    QValueSpaceObject* vSpace;
    QValueSpaceItem* vItem;
};


