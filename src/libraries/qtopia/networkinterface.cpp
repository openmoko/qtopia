/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "networkinterface.h"
#include "network.h"
#include "config.h"
#include <stdio.h>

/*!
    \class NetworkInterface networkinterface.h
    \brief The NetworkInterface class allows plugin network services.

    The Qtopia Network facilities are defined by the configurations
    in the Qtopia etc/network/ directory. For example, the generic
    DialUp.conf configuration looks like this (the actual file is
    encoded in UTF-8):

<pre>
[Info]
Name = Dial-up - PPP
Type = dialup
Layer = TCP/IP
[Help]
Help = For dial-up connections through a modem and plain telephone line.
Help[de] = Für Wählverbindungen mit Modem und Telefon.
[Properties]
Auto = 0
Device = ppp
CardType = serial
</pre>

    To add a network service that is merely a specific dialup service,
    you need only provide another configuration files, for example:

<pre>
[Info]
Name = SuperNet ISP
Type = dialup
Layer = TCP/IP
[Help]
Help = Connect to your local SuperNet(tm) point-of-presence.
[Properties]
Auto = 0
Device = ppp
CardType = serial
password = secret
phone = 555555555
speed = 115200
username = customer
</pre>

    However, to add a service beyond the scope of the existing
    services, you can define your own network service Type,
    by creating a NetworkInterface plugin.
*/

/*!
    \fn bool NetworkInterface::doProperties( QWidget *parent, Config& cfg )

    Prompts the user to manipulate the properties of the
    network service defined by \a cfg, using a modal dialog
    which has the given \a parent. Returns TRUE on success,
    of FALSE if the user cancels the interaction.
*/

/*!
    \fn bool NetworkInterface::create( Config& cfg )

    Performs any system state changes required for
    the network service defined by \a cfg. This is
    called only when the service is first added by
    the user.
*/

/*!
    \fn bool NetworkInterface::remove( Config& )

    Reverts any system state changes made in create().
*/

/*!
    \fn bool NetworkInterface::start( Config& cfg )

    Starts the network service defined by \a cfg,
    returning TRUE on success.

    There is no default implementation.
*/

/*!
    \fn bool NetworkInterface::stop( Config& cfg )

    Stops the network service defined by \a cfg,
    returning TRUE on success.

    There is no default implementation.
*/

/*!
    Returns the network device which is used for the connection.

    By default, the Properties.Device value in \a cfg is returned.
*/
QString NetworkInterface::device( Config& cfg ) const
{
    return cfg.readEntry("Device");
}

/*!
    Returns TRUE if the service defined by \a cfg is active.

    By default, the device() is searched for in /proc/net/dev.
*/
bool NetworkInterface::isActive( Config& cfg ) const
{    
    QString dev = device(cfg);
    if ( dev.isEmpty() )
	return FALSE;
    QString dev0 = dev+'0';

    FILE* f;
    f = fopen("/proc/net/dev", "r");
    if ( f ) {
	char line[1024];
	char devname[80];
	while ( fgets( line, 1024, f ) ) {
	    if ( sscanf(line," %[^:]:", devname)==1 )
	    {
		if ( devname == dev || devname == dev0 ) {
		    fclose(f);
#ifdef QWS
		    Network::writeProxySettings( cfg );
#endif
		    return TRUE;
		}
	    }
	}
	fclose(f);
    }
    return FALSE;
}

/*!
    Returns the pcmcia card type associated with this service.

    By default, the Properties.CardType value in \a cfg is returned.
*/
QString NetworkInterface::cardType( Config& cfg ) const
{
    return cfg.readEntry("CardType");
}

/*!
    Returns TRUE is the network service defined by \a cfg
    is available to be started.

    By default, the cardType() is searched for in the
    system pcmcia tables.
*/
bool NetworkInterface::isAvailable( Config& cfg ) const
{
    QString ct = cardType(cfg);
    if ( ct.isEmpty() )
	return FALSE;

    FILE* f = fopen("/var/run/stab", "r");
    if (!f) f = fopen("/var/state/pcmcia/stab", "r");
    if (!f) f = fopen("/var/lib/pcmcia/stab", "r");

    if ( f ) {
	char line[1024];
	char devtype[80];
	while ( fgets( line, 1024, f ) ) {
	    if ( sscanf(line,"%*d %s %*s", devtype )==1 )
	    {
		if ( ct == devtype ) {
		    fclose(f);
		    return TRUE;
		}
	    }
	}
	fclose(f);
    }

    return FALSE;
}

/*!
    \fn bool NetworkInterface::start( Config &cfg, const QString &password )

    Starts the network service defined by \a cfg, using the given
    \a password, returning TRUE on success.

    By default, the start() method without a password is called.
*/
bool NetworkInterface::start( Config& cfg, const QString& )
{
    return start(cfg);
}

/*!
    \fn bool NetworkInterface::needPassword( Config &cfg ) const
    Returns TRUE if the network service defined by \a cfg requires
    a password.
*/

bool NetworkInterface::needPassword( Config& ) const
{
    return FALSE;
}

/*!
    \fn QWidget* NetworkInterface::addStateWidget( QWidget *parent, Config &cfg ) const
    Returns a widget, with \a parent as its parent, that shows
    the state of the network service defined by \a cfg.

    By default, 0 is returned (no state widget).
*/
QWidget* NetworkInterface::addStateWidget( QWidget*, Config& ) const
{
    return 0;
}
