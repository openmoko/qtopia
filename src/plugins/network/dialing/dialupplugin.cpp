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

#include "dialupplugin.h"

#include <QPointer>
#include <qtopialog.h>
#include <qtopiaapplication.h>

DialupPlugin::DialupPlugin()
{
    //Load translation for libqtopianetwork
    QtopiaApplication::loadTranslations( "libqtopianetwork" );
}

DialupPlugin::~DialupPlugin()
{
    qLog(Network) << "Deleting DialupPlugin (" << instances.count()
        << " instances)";
    //cleanup all interface instances
    while ( !instances.isEmpty() )
    {
        QPointer<QtopiaNetworkInterface> impl = instances.takeFirst();
        if (impl)
            delete impl;
    }
}

QPointer<QtopiaNetworkInterface> DialupPlugin::network( const QString& confFile)
{
    qLog(Network) << "new Dialup interface instance requested";
    QPointer<QtopiaNetworkInterface> impl = new DialupImpl( confFile );
    instances.append(impl);

    return impl;
}

QtopiaNetwork::Type DialupPlugin::type() const
{
    //can handle all type of GPRS and analog dialup connection
    //pcmcia, internal device and specifically named devices
    //see dialup.cpp (DialupImpl::isAvailable())
    return ( QtopiaNetwork::Dialup |
             QtopiaNetwork::GPRS |
#ifdef QTOPIA_CELL
             QtopiaNetwork::PhoneModem |
#endif
             QtopiaNetwork::PCMCIA |
             QtopiaNetwork::NamedModem );
}

QTOPIA_EXPORT_PLUGIN( DialupPlugin );
