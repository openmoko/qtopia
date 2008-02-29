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

#include "taskbarsettings.h"
#include <qtopia/config.h>
#include <qtopia/pluginloader.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/taskbarappletinterface.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qdir.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qheader.h>
#include <stdlib.h>

TaskbarSettings::TaskbarSettings( QWidget *parent, const char *name, bool modal, WFlags f )
    : TaskbarSettingsBase( parent, name, modal, f|WStyle_ContextHelp ), appletsChanged(FALSE)
{
    appletListView->header()->hide();
    connect( appletListView, SIGNAL(clicked(QListViewItem*,const QPoint &,int)), this, SLOT(itemClicked(QListViewItem *,const QPoint &,int)) );
    appletListView->installEventFilter( this );
    init();
}

void TaskbarSettings::init()
{
    Config cfg( "Taskbar" );
    cfg.setGroup( "Applets" );
    QStringList exclude = cfg.readListEntry( "ExcludeApplets", ',' );

    PluginLoader loader( "applets" );
    QStringList list = loader.list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	QString name;
	QPixmap icon;
#ifndef Q_OS_WIN32
	name = (*it).mid(3);
	int sep = name.find( ".so" );
#else
	name = (*it);
	int sep = name.find( ".dll" );
#endif
	if ( sep > 0 )
	    name.truncate( sep );
	sep = name.find( "applet" );
#ifndef Q_OS_WIN32
	if ( sep == (int)name.length() - 6 )
	    name.truncate( sep );
#else
	if (sep >= 0){
	    name.truncate( sep );
	}else{
	    qDebug("No applet in %s", (char*)name.latin1());
	}
#endif
	name[0] = name[0].upper();
	QCheckListItem *item;
	item = new QCheckListItem( appletListView, name,
		QCheckListItem::CheckBox );
	if ( !icon.isNull() )
	    item->setPixmap( 0, icon );
	if ( exclude.find( *it ) == exclude.end() )
	    item->setOn( TRUE );
	applets[*it] = item;
    }

    cfg.setGroup( "Menu" );
    menu_launcher_tabs->setChecked(cfg.readBoolEntry("LauncherTabs",TRUE));
}

void TaskbarSettings::itemClicked( QListViewItem *i, const QPoint &p, int )
{
    if ( i ) {
	appletsChanged = TRUE;
	if ( appletListView->mapFromGlobal(p).x() > 20 ) {
	    QCheckListItem *ci = (QCheckListItem *)i;
	    ci->setOn( !ci->isOn() );
	}
    }
}

bool TaskbarSettings::eventFilter( QObject *o, QEvent *e )
{
    if ( o == appletListView && e->type() == QEvent::KeyPress ) {
	if ( ((QKeyEvent*)e)->key() == Key_Space )
	    appletsChanged = TRUE;
    }

    return FALSE;
}

void TaskbarSettings::accept()
{
    {
	Config cfg("Taskbar");
	cfg.setGroup( "Applets" );
	if ( appletsChanged ) {
	    QStringList exclude;
	    QMap<QString,QCheckListItem*>::Iterator it;
	    for ( it=applets.begin(); it!=applets.end(); ++it) {
		if ( !(*it)->isOn() )
		    exclude << it.key();
	    }
	    cfg.writeEntry( "ExcludeApplets", exclude, ',' );
	}
	cfg.writeEntry( "SafeMode", FALSE );
	cfg.setGroup( "Menu" );
	if ( menu_launcher_tabs->isChecked() != cfg.readBoolEntry("LauncherTabs",TRUE) ) {
	    appletsChanged = TRUE; 
	    cfg.writeEntry("LauncherTabs",menu_launcher_tabs->isChecked());
	}
    }

    if ( appletsChanged ) {
	QCopEnvelope( "QPE/TaskBar", "reloadApplets()" );
	appletsChanged = FALSE;
    }

    QDialog::accept();
}

void TaskbarSettings::done( int r )
{
    QDialog::done(r);
    close();
}
