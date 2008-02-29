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

    if ( m ) {
	qDebug( "Use QPEApplication's PluginLibraryManager" );
    } else {
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

