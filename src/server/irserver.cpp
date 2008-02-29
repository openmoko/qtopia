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

#include <qtopia/qlibrary.h>
#include <qtopia/qpeapplication.h>

#include <qtranslator.h>
#include <stdlib.h>

#include "obexinterface.h"

#include <qdir.h>

IrServer::IrServer( QObject *parent, const char *name )
  : QObject( parent, name ), lib(0), obexIface(0)
{
    QString path = QPEApplication::qpeDir() + "plugins/obex/";
#ifndef Q_OS_WIN32
    QDir dir( path, "lib*.so" );
#else
    QDir dir( path, "*.dll");
#endif
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	ObexInterface *iface = 0;
	QLibrary *trylib = new QLibrary( path + *it );
	if ( trylib->queryInterface( IID_ObexInterface, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
	    obexIface = iface;
	    lib = trylib;
	    qDebug("found obex lib" );
	    QString lang = getenv( "LANG" );
	    QTranslator * trans = new QTranslator(qApp);
	    QString type = (*it).left( (*it).find(".") );
	    QString tfn = QPEApplication::qpeDir()+"i18n/"+lang+"/"+type+".qm";
	    qDebug("tr for obex: %s", tfn.latin1() );
	    if ( trans->load( tfn ))
		qApp->installTranslator( trans );
	    else
		delete trans;

	    break;
	} else {
	    delete trylib;
	}
    }
    if ( !lib )
	qDebug("could not load IR plugin" );
}

IrServer::~IrServer()
{
    if ( obexIface )
	obexIface->release();
    if ( lib )
	lib->unload();
    delete lib;
}

