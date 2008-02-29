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

#include "networksettings.h"
#include "addnetworksetting.h"

#include <qtopia/networkinterface.h>
#include <qtopia/network.h>
#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/qlibrary.h>
#include <qtopia/qpemessagebox.h>


#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qdict.h>
#include <qobjectlist.h>

/* 
 *  Constructs a NetworkConnections which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
NetworkSettings::NetworkSettings( QWidget* parent,  const char* name, WFlags fl )
    : NetworkSettingsBase( parent, name, fl )
{
    emptyString = tr("<p>There are no running network services.");
    notEmptyString = tr("<p>This is the state of your running network services.");
    loadSettings();

    connect(add, SIGNAL(clicked()), this, SLOT(addSetting()));
    connect(remove, SIGNAL(clicked()), this, SLOT(removeSetting()));
    connect(props, SIGNAL(clicked()), this, SLOT(doProperties()));
    connect(settings, SIGNAL(selectionChanged()), this, SLOT(serviceSelected()) );

    remove->setEnabled( false );
    props->setEnabled( false );

#ifdef QWS
    Network::connectChoiceChange(this,SLOT(updateCurrent()));
#endif

    updateCurrent();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
NetworkSettings::~NetworkSettings()
{
    // no need to delete child widgets, Qt does it all for us
}

void NetworkSettings::loadSettings()
{
#ifdef QWS
    filelist = Network::choices(settings);
#endif
}

void NetworkSettings::addSetting()
{
    AddNetworkSetting a;
    a.showMaximized();
    if ( a.exec() ) {
	QString basename = QFileInfo(a.selectedFile()).baseName();
	QDir settingsdir(Network::settingsDir());
	QString uniq = "";
	int n=0;
	QString filename;
	do {
	    filename = settingsdir.filePath(basename + uniq + ".conf");
	    uniq = QString::number(n++);
	} while (QFile::exists(filename));

	// copy
	{
	    QFile i(a.selectedFile());
	    if ( !i.open(IO_ReadOnly) )
		return;
	    QFile o(filename);
	    if ( !o.open(IO_WriteOnly) )
		return;
	    o.writeBlock(i.readAll());
	}
	bool ok = TRUE;
	{
	    Config cfg(filename, Config::File);
	    QFile::remove(filename);
	    cfg.setGroup("Help");
	    cfg.clearGroup();
	    cfg.setGroup("Info");
	    QString type = cfg.readEntry("Type");

	    NetworkInterface* plugin;
	    if ( cfg.readNumEntry("Unique",0) && n > 1 ) {
		QMessageBox::information(this,tr("Error"),
		    tr("<p>Only one %1 can be added. Remove the current one, "
			"or change its Properties").arg(cfg.readEntry("Name")));
		plugin = 0;
	    } else {
#ifdef QWS
		plugin = Network::loadPlugin(type);
#endif
	    }

	    if ( plugin ) {
		if ( (ok = plugin->create(cfg)) ) {
		    cfg.setGroup("Properties");
		    if ( !plugin->doProperties(this, cfg) ) {
			plugin->remove(cfg);
			plugin = 0;
			ok = FALSE;
		    }
		}
	    } else {
		ok = FALSE;
	    }
	    if ( ok )
		cfg.write(); // since we delete it after load
	}
	if ( ok ) {
	    loadSettings();
#ifndef QT_NO_COP
	    QCopEnvelope("QPE/Network", "choicesChanged()");
#endif
	} else {
	    QFile::remove(filename);
	}
    }
}

void NetworkSettings::serviceSelected()
{
    bool sel = false;

    QStringList::ConstIterator it = filelist.begin();
    for (QListBoxItem* i=settings->firstItem(); i; i=i->next(), ++it) {
	if ( i->selected() ) {
	    sel = true;
	    break;
	}
    }

    remove->setEnabled( sel );
    props->setEnabled( sel );
}


void NetworkSettings::removeSetting()
{
    bool ch=FALSE;
    QStringList::ConstIterator it = filelist.begin();
    for (QListBoxItem* i=settings->firstItem(); i; i=i->next(), ++it) {
	if ( i->selected() && QPEMessageBox::confirmDelete( 0, 
                    NetworkSettings::tr( "Network" ), i->text()) ) {
	    removeSetting(*it);
	    ch = TRUE;
	}
    }
    if ( ch ) {
	loadSettings();
#ifndef QT_NO_COP
	QCopEnvelope("QPE/Network", "choicesChanged()");
#endif
	serviceSelected();
    }
}

void NetworkSettings::removeSetting(const QString& cfgfile)
{
    {
	Config cfg(cfgfile, Config::File);
	cfg.setGroup("Info");
	QString type = cfg.readEntry("Type");
#ifdef QWS
	NetworkInterface* plugin = Network::loadPlugin(type);
	if ( plugin ) {
	    if ( !plugin->remove(cfg) )
		return;
	}
#endif
    }
    QFile::remove(cfgfile);
}

void NetworkSettings::doProperties()
{
    QStringList::ConstIterator it = filelist.begin();
    for (QListBoxItem* i=settings->firstItem(); i; i=i->next(), ++it) {
	if ( i->selected() ) {
	    doProperties(*it);
	    loadSettings();
#ifndef QT_NO_COP
	    QCopEnvelope("QPE/Network", "choicesChanged()");
#endif
	    return;
	}
    }
}

void NetworkSettings::doProperties(const QString& cfgfile)
{
    Config cfg(cfgfile, Config::File);
    cfg.setGroup("Info");
    QString type = cfg.readEntry("Type");
#ifdef QWS
    NetworkInterface* plugin = Network::loadPlugin(type);
    if ( plugin ) {
	cfg.setGroup("Properties");
	plugin->doProperties(this, cfg);
	updateCurrent();
    }
#endif
}

void NetworkSettings::updateCurrent()
{
    states->hide();
    const QObjectList *l = states->children();
    if ( l ) {
	start: {
	    QObjectListIt i(*l);
	    while (i.current()) {
		if (i.current()->isWidgetType()) {
		    delete i.current();
		    goto start;
		}
		++i;
	    }
	}
    }
    delete states->layout();
#ifdef QWS
    Network::addStateWidgets(states);
#endif
    states->show();
}
