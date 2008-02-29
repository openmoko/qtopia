/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "pluginloaderlib_p.h"
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qdir.h>
#include <qdict.h>
#include <qtimer.h>

#include <stdlib.h>

//===========================================================================

static PluginLibraryManager *manager = 0;

/*
 * This is nasty.  If we are running a > 1.6 libqpe then there will be
 * a PluginLibraryManager already created and we want to use that one,
 * but we cannot assume the symbol exists since libqpe 1.5 will not have it.
 * -> Abuse the signal/slot mechanism to get the manager.
 */
class PluginLibraryManagerFinder : public QObject
{
    Q_OBJECT
public:
    PluginLibraryManager *getManager();

signals:
    void getInstance( PluginLibraryManager ** );
};

PluginLibraryManager *PluginLibraryManagerFinder::getManager()
{
    static PluginLibraryManager *m = 0;
    if ( m)
	return m;
    connect( this, SIGNAL(getInstance(PluginLibraryManager**)),
	     qApp, SLOT(pluginLibraryManager(PluginLibraryManager**)) );
    emit getInstance( &m );
    disconnect( this, SIGNAL(getInstance(PluginLibraryManager**)),
	     qApp, SLOT(pluginLibraryManager(PluginLibraryManager**)) );

    if ( !m ) {
	qDebug( "Create our own PluginLibraryManager" );
	m = new PluginLibraryManager;
    }
    return m;
}

PluginLibraryManager *pluginLibraryManagerInstance()
{
    if ( !manager ) {
	PluginLibraryManagerFinder *finder = new PluginLibraryManagerFinder;
	manager = finder->getManager();
	delete finder;
    }

    return manager;
}

#include "pluginloaderlib.moc"

