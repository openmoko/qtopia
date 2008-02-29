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
#include <qpe/config.h>
#include <qpe/qlibrary.h>
#include <qpe/qpeapplication.h>
#include <qpe/taskbarappletinterface.h>
#include <qpe/qcopenvelope_qws.h>
#include <qdir.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qheader.h>
#include <stdlib.h>

TaskbarSettings::TaskbarSettings( QWidget *parent, const char *name, bool modal, WFlags f )
    : TaskbarSettingsBase( parent, name, modal, f|WStyle_ContextHelp ), appletsChanged(FALSE)
{
    appletListView->header()->hide();
    connect( appletListView, SIGNAL(clicked(QListViewItem*)), this, SLOT(appletChanged()) );
    init();
}

void TaskbarSettings::init()
{
    Config cfg( "Taskbar" );
    cfg.setGroup( "Applets" );
    QStringList exclude = cfg.readListEntry( "ExcludeApplets", ',' );

    QString path = QPEApplication::qpeDir() + "/plugins/applets";
    QDir dir( path, "lib*.so" );
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	QString name;
	QPixmap icon;
	TaskbarNamedAppletInterface *iface = 0;
	QLibrary *lib = new QLibrary( path + "/" + *it );
	lib->queryInterface( IID_TaskbarNamedApplet, (QUnknownInterface**)&iface );
	if ( iface ) {
	    QString lang = getenv( "LANG" );
	    QTranslator * trans = new QTranslator(qApp);
	    QString type = (*it).left( (*it).find(".") );
	    QString tfn = QPEApplication::qpeDir()+"/i18n/"+lang+"/"+type+".qm";
	    if ( trans->load( tfn ))
		qApp->installTranslator( trans );
	    else
		delete trans;
	    name = iface->name();
	    icon = iface->icon();
	    iface->release();
	    lib->unload();
	} else {
	    delete lib;
	    name = (*it).mid(3);
	    int sep = name.find( ".so" );
	    if ( sep > 0 )
		name.truncate( sep );
	    sep = name.find( "applet" );
	    if ( sep == (int)name.length() - 6 )
		name.truncate( sep );
	    name[0] = name[0].upper();
	}
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

void TaskbarSettings::appletChanged()
{
    appletsChanged = TRUE;
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
