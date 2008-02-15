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

#ifndef WIRELESSNETWORK
#define WIRELESSNETWORK

#include <QVariant>
#include <QHash>

class WirelessNetwork
{
public:
    WirelessNetwork();
    ~WirelessNetwork();

    enum ParameterType {
        Protocol = 0,       //SIOCGIWNAME
        AP,             //SIOCGIWAP
        ESSID,          //SIOCGIWESSID
        Mode,           //SIOCGIWMODE
        NWID,           //SIOCGIWNWID
        BitRate,        //SIOCGIWRATE
        Frequency,      //SIOCGIWFREQ
        Channel,        //based on SIOCGIWFREQ
        Encoding,       //SIOCGIWENCODE
        Security,       //based on SIOCGIENCODE
        Quality,        //IWEVQUAL
        Signal,         //IWEVQUAL
        Noise,          //IWEVQUAL
        Custom,         //IWEVCUSTOM
    };

    //void dump() const;
    bool isValid() const;
    void setData( ParameterType t, const QVariant& data );
    QVariant data( ParameterType t ) const;

    void addCustomData( const QVariant& data );
    QList<QVariant> customData() const;

private:
    QHash<int,QVariant> dataMap;
    QList<QVariant> custom;
};
#endif //WIRELESSNETWORK
