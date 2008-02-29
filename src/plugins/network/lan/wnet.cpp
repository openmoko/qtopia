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

#include "wnet.h"
#include <QDebug>

/*!
  \internal
  \class WirelessNetwork
  \brief This class is a small container class for wireless LAN parameter.
  */

WirelessNetwork::WirelessNetwork()
{
    dataMap.clear();
    custom.clear();
}

WirelessNetwork::~WirelessNetwork()
{
}

void WirelessNetwork::setData( ParameterType t, const QVariant& data )
{
    dataMap.insert( (int)t, data );
}

QVariant WirelessNetwork::data( ParameterType t ) const
{
    return dataMap.value( (int)t );
}

void WirelessNetwork::addCustomData( const QVariant& data )
{
    custom.append( data );
}

QList<QVariant> WirelessNetwork::customData() const
{
    return custom;
}

bool WirelessNetwork::isValid() const
{
    return ( dataMap.size() > 0 );
}

/*
void WirelessNetwork::dump() const
{
    QList<int> keys = dataMap.keys();
    foreach( int s, keys )
    {
        qLog(Network) << s << dataMap[s];
    }
    foreach( QVariant a, custom )
        qDebug() << a;
}*/
