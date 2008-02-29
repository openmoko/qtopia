/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "bluetoothplugin.h"
#include "bluetoothimpl.h"

#include <QPointer>
#include <qtopialog.h>
#include <qtopiaapplication.h>

BluetoothPlugin::BluetoothPlugin()
{
    //Load translation for libqtopianetwork
    QtopiaApplication::loadTranslations( "libqtopianetwork" );
}

BluetoothPlugin::~BluetoothPlugin()
{
    qLog(Network) << "Deleting BluetoothPlugin (" << instances.count()
        << " instances)";
    //cleanup all interface instances
    while ( !instances.isEmpty() )
    {
        QPointer<QtopiaNetworkInterface> impl = instances.takeFirst();
        if (impl)
            delete impl;
    }
}

QPointer<QtopiaNetworkInterface> BluetoothPlugin::network( const QString& confFile)
{
    qLog(Network) << "new Bluetooth interface instance requested";
    QPointer<QtopiaNetworkInterface> impl = new BluetoothImpl( confFile );
    instances.append(impl);

    return impl;
}

QtopiaNetwork::Type BluetoothPlugin::type() const
{
    return ( QtopiaNetwork::Bluetooth| QtopiaNetwork::BluetoothDUN );
}

QTOPIA_EXPORT_PLUGIN( BluetoothPlugin );
