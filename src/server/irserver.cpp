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


#include "irserver.h"

#include <qtopia/pluginloader.h>
#include <qtopia/qpeapplication.h>

#include <qtranslator.h>
#include <stdlib.h>

#include "obexinterface.h"

#include <qdir.h>

IrServer::IrServer( QObject *parent, const char *name )
  : QObject( parent, name ), obexIface(0)
{
    loader = new PluginLoader( "obex" );
    QStringList list = loader->list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	ObexInterface *iface = 0;
	if ( loader->queryInterface( *it, IID_ObexInterface, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
	    obexIface = iface;
	    qDebug("found obex lib" );
	    break;
	}
    }
    if ( !obexIface )
	qDebug("could not load IR plugin" );
}

IrServer::~IrServer()
{
    if ( obexIface )
	loader->releaseInterface( obexIface );
    delete loader;
}

