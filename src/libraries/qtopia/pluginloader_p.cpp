/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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


#ifndef PLUGINLOADER_P_H
#define PLUGINLOADER_P_H

#define PLUGINLOADER_INTERN

#include "pluginloader_p.h"
#include <signal.h>

#include "../qtopia1/pluginloader.cpp"

static PluginLibraryManager *manager = 0;

static void cleanupPluginLibraryManager()
{
    delete manager;
}

static void handleCrash( int s )
{
#ifndef Q_OS_WIN32
     if ( s != SIGSEGV && s != SIGBUS )
 	return;
#else
    if ( s != SIGSEGV )
	return;
#endif

    if ( qApp->type() == QApplication::GuiServer ) {
	Config cfg( "PluginLoader" );
	cfg.setGroup( "Global" );
	QString pmode = cfg.readEntry( "Mode", "Normal" );
	if ( pmode == "MaybeSafe" )
	    cfg.writeEntry( "Mode", "Safe" );
	else
	    cfg.writeEntry( "Mode", "MaybeSafe" );
    }

    signal( s, SIG_DFL );
    abort();
}

PluginLibraryManager *pluginLibraryManagerInstanceIntern()
{
    if ( !manager ) {
	qDebug( "Create pluginlibman in libqpe" );
	manager = new PluginLibraryManager;
	qAddPostRoutine( cleanupPluginLibraryManager );
	if ( qApp->type() == QApplication::GuiServer )
	    signal( SIGSEGV, handleCrash );
    }

    return manager;
}

#endif
