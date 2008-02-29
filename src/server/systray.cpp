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

#include <qpe/qpeapplication.h>
#include <qpe/qlibrary.h>
#include <qpe/config.h>

#include <qlayout.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qtranslator.h>

#include "quicklauncher.h"
#include "systray.h"

#include <stdlib.h>

#ifdef SINGLE_APP
#include "clockappletimpl.h"
#endif

SysTray::SysTray( QWidget *parent ) : QFrame( parent ), layout(0)
{
    safety_tid = 0;
    //setFrameStyle( QFrame::Panel | QFrame::Sunken );
    loadApplets();
}

SysTray::~SysTray()
{
    clearApplets();
}

static int compareAppletPositions(const void *a, const void *b)
{
    const TaskbarApplet* aa = *(const TaskbarApplet**)a;
    const TaskbarApplet* ab = *(const TaskbarApplet**)b;
    int d = ab->iface->position() - aa->iface->position();
    if ( d ) return d;
    return QString::compare(ab->library->library(),aa->library->library());
}

void SysTray::loadApplets()
{
    hide();
    clearApplets();
    addApplets();
}

void SysTray::clearApplets()
{
#ifndef SINGLE_APP
    QValueList<TaskbarApplet*>::Iterator mit;
    for ( mit = appletList.begin(); mit != appletList.end(); ++mit ) {
	(*mit)->iface->release();
	(*mit)->library->unload();
	delete (*mit)->library;
	delete (*mit);
    }
#endif
    appletList.clear();
    if ( layout )
	delete layout;
    layout = new QHBoxLayout( this, 0, 1 );
    layout->setAutoAdd(TRUE);
}

void SysTray::addApplets()
{
    hide();
#ifndef SINGLE_APP
    Config cfg( "Taskbar" );
    cfg.setGroup( "Applets" );
    bool safe = cfg.readBoolEntry("SafeMode",FALSE);
    if ( safe && !safety_tid ) {
	QMessageBox::warning(this, tr("Safe Mode"), tr("<P>A system startup error occurred, "
		"and the system is now in Safe Mode. "
		"Applets are not loaded in Safe Mode. "
		"You can use the Taskbar settings to "
		"enable applets that do not cause system error."), QMessageBox::Ok, 0);
	return;
    }
    cfg.writeEntry("SafeMode",TRUE);
    cfg.write();
    QStringList exclude = cfg.readListEntry( "ExcludeApplets", ',' );
    QStringList faulty;

    QString path = QPEApplication::qpeDir() + "/plugins/applets";
    QDir dir( path, "lib*.so" );
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    int napplets=0;
    TaskbarApplet **applets = new TaskbarApplet* [list.count()];
    for ( it = list.begin(); it != list.end(); ++it ) {
	if ( exclude.find( *it ) != exclude.end() )
	    continue;
	TaskbarAppletInterface *iface = 0;
	QLibrary *lib = new QLibrary( path + "/" + *it );
	if ( lib->queryInterface( IID_TaskbarApplet, (QUnknownInterface**)&iface ) == QS_OK ) {
	    TaskbarApplet *applet = new TaskbarApplet;
	    applets[napplets++] = applet;
	    applet->library = lib;
	    applet->iface = iface;
	} else {
	    exclude += *it;

	    // Same as Taskbar settings uses
	    QString name = (*it).mid(3);
            int sep = name.find( ".so" );
            if ( sep > 0 )
                name.truncate( sep );
            sep = name.find( "applet" );
            if ( sep == (int)name.length() - 6 )
                name.truncate( sep );
            name[0] = name[0].upper();
	    faulty += name; 

	    delete lib;
	}
    }
    if ( faulty.count() ) {
	QMessageBox::warning(this, tr("Applet Error"), tr("<P>The following applets could not be loaded. "
		"They have been disabled. You can use the Taskbar settings to attempt to "
		"re-enable them.<P><b>%1</b>.").arg(faulty.join("</b>, <b>")), QMessageBox::Ok, 0);
    }
    cfg.writeEntry( "ExcludeApplets", exclude, ',' );
    qsort(applets,napplets,sizeof(applets[0]),compareAppletPositions);
    while (napplets--) {
	TaskbarApplet *applet = applets[napplets];
	applet->applet = applet->iface->applet( this );
	appletList.append(applet);
	QString lang = getenv( "LANG" );
	QTranslator * trans = new QTranslator(qApp);
	QString type = (*it).left( (*it).find(".") );
	QString tfn = QPEApplication::qpeDir()+"/i18n/"+lang+"/"+type+".qm";
	if ( trans->load( tfn ))
	    qApp->installTranslator( trans );
	else
	    delete trans;
    }
    delete [] applets;
#else
    TaskbarApplet applet;
    applet.iface = new ClockAppletImpl();
    applet.applet = applet.iface->applet( this );
    appletList.append( a );
#endif
    show();

    if ( !safety_tid )
	safety_tid = startTimer(5000);
}

void SysTray::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == safety_tid ) {
	Config cfg( "Taskbar" );
	cfg.setGroup( "Applets" );
	cfg.writeEntry( "SafeMode", FALSE );
	killTimer(safety_tid);
	safety_tid = 0;
    }
}
