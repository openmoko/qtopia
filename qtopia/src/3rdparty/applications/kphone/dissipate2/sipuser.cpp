/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "sipclient.h"
#include "sipuser.h"

SipUser::SipUser( SipClient *parent, QString fullname, QString username, QString athostname )
{
	client = parent;
	myuri.setFullname( fullname );
	myuri.setUsername( username );
	myuri.setHostname( athostname );
	client->setUser( this );
}

SipUser::SipUser( SipClient *parent, const SipUri &inituri )
{
	client = parent;
	myuri = inituri;
	client->setUser( this );
}

SipUser::~SipUser( void )
{
}

void SipUser::addServer( SipRegister *server )
{
	servers.append( server );
}

void SipUser::removeServer( SipRegister *server )
{
	servers.remove( server );
}

void SipUser::setUri( const SipUri &newuri )
{
	myuri = newuri;
}
