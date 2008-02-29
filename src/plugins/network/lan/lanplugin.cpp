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

#include "lanplugin.h"

#include <QPointer>
#include <qtopialog.h>
#include <qtopiaapplication.h>

LanPlugin::LanPlugin()
{
    //Load translation for libqtopianetwork
    QtopiaApplication::loadTranslations( "libqtopianetwork" );
}

LanPlugin::~LanPlugin()
{
    qLog(Network) << "Deleting LanPlugin (" << instances.count()
        << " instances)";
    //cleanup all interface instances
    while ( !instances.isEmpty() )
    {
        QPointer<QtopiaNetworkInterface> impl = instances.takeFirst();
        if (impl)
            delete impl;
    }
}

QPointer<QtopiaNetworkInterface> LanPlugin::network( const QString& confFile)
{
    qLog(Network) << "new Lan interface instance requested -> " << confFile;
    QPointer<QtopiaNetworkInterface> impl = new LanImpl( confFile );
    instances.append(impl);

    return impl;
}

QtopiaNetwork::Type LanPlugin::type() const
{
    return ( QtopiaNetwork::LAN |
#ifndef NO_WIRELESS_LAN
                QtopiaNetwork::WirelessLAN |
#endif
                QtopiaNetwork::PCMCIA );
}

QByteArray LanPlugin::customID() const
{
    return QByteArray();
}

QTOPIA_EXPORT_PLUGIN( LanPlugin );
