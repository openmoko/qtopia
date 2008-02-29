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

#include "networksettings.h"
#include "addnetworksetting.h"

#include <qtopia/networkinterface.h>
#include <qtopia/network.h>
#include <qtopia/contextmenu.h>
#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/qlibrary.h>
#include <qtopia/qpemessagebox.h>
#ifdef QTOPIA_PHONE
#include <qtopia/phone/smsmessage.h>
#include <qtopia/wap/wsppdu.h>
#include <qtopia/wap/otareader.h>
#endif


#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qaction.h>
#include <qdict.h>
#include <qobjectlist.h>
#include <qbuffer.h>

/* 
 *  Constructs a NetworkConnections which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
NetworkSettings::NetworkSettings( QWidget* parent,  const char* name, WFlags fl ) :
    QWidget(parent,name,fl)
{
    firstshow = TRUE;
    tid_close_self = 0;
    emptyString = tr("<p>There are no network services running.");
    notEmptyString = tr("<p>This is the state of the network services running:");

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setAutoAdd(TRUE);

#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu = new ContextMenu(this);

    a_add = new QAction( tr("Add..."), QString::null, 0, this );
    connect( a_add, SIGNAL( activated() ), this, SLOT( addSetting() ) );
    a_add->addTo(contextMenu);

    a_remove = new QAction( tr("Remove"), QString::null, 0, this );
    connect( a_remove, SIGNAL( activated() ), this, SLOT( removeSetting() ) );
    a_remove->addTo(contextMenu);

    a_props = new QAction( tr("Properties..."), QString::null, 0, this );
    connect( a_props, SIGNAL( activated() ), this, SLOT( doProperties() ) );
    a_props->addTo(contextMenu);

    a_remove->setEnabled( false );
    a_props->setEnabled( false );

    settings = new QListBox(this);
    settings->setFrameStyle(QFrame::NoFrame);
    states = 0; // Not used
    status = new QLabel(this);

    connect(settings, SIGNAL(selected(int)), this, SLOT(serviceSelected()) );
    connect(settings, SIGNAL(selectionChanged()), this, SLOT(updateActions()) );

    setCaption(tr("Internet"));

    QCopChannel* netChannel = new QCopChannel( "QPE/Network", this );
    connect( netChannel, SIGNAL(received(const QCString&,const QByteArray&)),
             this, SLOT(netMessage(const QCString&,const QByteArray&)) );
#else
    base = new NetworkSettingsBase(this);

    connect(base->add, SIGNAL(clicked()), this, SLOT(addSetting()));
    connect(base->remove, SIGNAL(clicked()), this, SLOT(removeSetting()));
    connect(base->props, SIGNAL(clicked()), this, SLOT(doProperties()));

    base->remove->setEnabled( false );
    base->props->setEnabled( false );

    settings = base->settings;
    states = base->states;

    connect(settings, SIGNAL(selectionChanged()), this, SLOT(serviceSelected()) );

    setCaption(base->caption());
#endif

    loadSettings();

#ifdef QTOPIA_PHONE
    QObject::connect
	( qApp, SIGNAL( appMessage(const QCString&,const QByteArray&) ),
	  this, SLOT( appMessage(const QCString&,const QByteArray&) ) );
#endif

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
    int i = settings->currentItem();
    if ( i < 0 ) i = 0;
    filelist = Network::choices(settings);
    if ( settings->count() )
	settings->setCurrentItem(i);
#endif
}

void NetworkSettings::addSetting()
{
    addSetting(FALSE);
}

void NetworkSettings::addSetting(bool first)
{
    AddNetworkSetting a(first, this);
    a.showMaximized();
    if ( a.exec() )
	addSetting(a.selectedFile(),TRUE);
}

QString NetworkSettings::addSetting(const QString& baseconfig, bool edit)
{
    QString basename = QFileInfo(baseconfig).baseName();
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
	QFile i(baseconfig);
	if ( !i.open(IO_ReadOnly) )
	    return QString::null;
	QFile o(filename);
	if ( !o.open(IO_WriteOnly) )
	    return QString::null;
	o.writeBlock(i.readAll());
    }
    bool ok = TRUE;
    {
	Config cfg(filename, Config::File);
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
		if ( edit ) {
		    cfg.setGroup("Properties");
		    if ( !plugin->doProperties(this, cfg) ) {
			plugin->remove(cfg);
			plugin = 0;
			ok = FALSE;
		    }
		}
	    }
	} else {
	    ok = FALSE;
	}
    }
    if ( ok ) {
	loadSettings();
#ifndef QT_NO_COP
	QCopEnvelope("QPE/Network", "choicesChanged()");
#endif
	return filename;
    } else {
	QFile::remove(filename);
	return QString::null;
    }
}

void NetworkSettings::updateActions()
{
#ifdef QTOPIA_PHONE
    bool sel = settings->currentItem() != -1;
    a_remove->setEnabled( TRUE );
    a_props->setEnabled( sel );
#endif
}

void NetworkSettings::serviceSelected()
{
    bool sel = false;
    QString selfile;

    QStringList::ConstIterator it = filelist.begin();
    for (QListBoxItem* i=settings->firstItem(); i; i=i->next(), ++it) {
	if ( i->selected() ) {
	    sel = true;
	    selfile = *it;
	    break;
	}
    }

#ifdef QTOPIA_PHONE
    if ( sel ) {
	Config cfg(selfile,Config::File);
	cfg.setGroup("Info");
	QString type = cfg.readEntry("Type");
	NetworkInterface* plugin = Network::loadPlugin(type);
	if ( plugin ) {
	    if ( plugin->isActive(cfg) ) {
		Network::stop();
	    } else if ( plugin->isAvailable(cfg) ) {
		Network::start(selfile);
	    } else {
		QMessageBox::information(this,tr("Error"),
		    tr("<p>Service cannot be explicitly started."));
	    }
	} else {
	    qWarning("No plugin type '%s' (%s)",type.latin1(),selfile.latin1());
	}
    }
#else
    base->remove->setEnabled( sel );
    base->props->setEnabled( sel );
#endif
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
#ifdef QTOPIA_PHONE
	updateActions();
#else
	serviceSelected();
#endif
    }
}

void NetworkSettings::removeSetting(const QString& cfgfile)
{
#ifdef QWS
    Config cfg(cfgfile, Config::File);
    cfg.setGroup("Info");
    QString type = cfg.readEntry("Type");
    NetworkInterface* plugin = Network::loadPlugin(type);
    if ( plugin ) {
	if ( !plugin->remove(cfg) )
	    return;
    }
#endif
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
#ifdef QWS
    Config cfg(cfgfile, Config::File);
    cfg.setGroup("Info");
    QString type = cfg.readEntry("Type");
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
#ifdef QTOPIA_PHONE
    QStringList::ConstIterator it = filelist.begin();
    int ind=0;
    for (QListBoxItem* i=settings->firstItem(); i; i=i->next(), ++it, ++ind) {
	Config cfg(*it, Config::File);
	cfg.setGroup("Info");
	QString type = cfg.readEntry("Type");
	NetworkInterface* plugin = Network::loadPlugin(type);
	if ( plugin ) {
	    QString x;
            QString name = cfg.readEntry("Name");
	    /* Not meaningful to user
	    if ( plugin->isAvailable(cfg) )
		x += tr(" (available)");
	    */
	    if ( plugin->isActive(cfg) )
		x += tr(" (active)");
	    if ( !x.isEmpty() )
		settings->changeItem(
		    Resource::loadPixmap("Network/" + type+ '/' + type),
		    name+x,
		    ind);
	}
    }
#else
    bool v = states->isVisible();

    if ( v ) states->hide();
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
    if ( v ) states->show();
#endif
}

void NetworkSettings::show()
{
    QWidget::show();
    if ( firstshow ) {
	firstshow = FALSE;
	if ( settings->count()==0 ) {
	    addSetting(TRUE);
	    if ( settings->count()==0 ) {
		// close (from inside show(), so be careful)
		tid_close_self = startTimer(0);
	    }
	}
    }
}

void NetworkSettings::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == tid_close_self ) {
	killTimer(tid_close_self);
	tid_close_self=0;
	close();
	firstshow = TRUE;
    }
}

void NetworkSettings::netMessage( const QCString& 
#ifdef QTOPIA_PHONE
    msg, const QByteArray& data
#else
    , const QByteArray&
#endif
 )
{
#ifdef QTOPIA_PHONE
    if (msg == "progress(QString,int)" ) {
	QDataStream stream(data,IO_ReadOnly);
        QString p;
        int flag;
        stream >> p >> flag;
	status->setText(p);
    }
#endif
}

void NetworkSettings::appMessage( const QCString& 
#ifdef QTOPIA_PHONE
    msg, const QByteArray& data
#else
    , const QByteArray&
#endif
        )
{
#ifdef QTOPIA_PHONE
    if (msg == "smsApplicationDatagram(SMSMessage)") {
	QDataStream stream(data,IO_ReadOnly);
	SMSMessage smsMessage;
	stream >> smsMessage;
	if (smsMessage.destinationPort() == 2948 ||	// WAP/OMA-style
	    smsMessage.destinationPort() == 49999) {	// Nokia-style

	    QBuffer pushpdu(smsMessage.applicationData());
	    pushpdu.open(IO_ReadOnly);
	    WspPduDecoder decoder(&pushpdu);
	    WspPush push = decoder.decodePush();
	    const WspField *field = push.header("Content-Type");

	    QString tmp = (field ? field->value : QString());
	    OtaReader *reader = 0;
	    if (tmp.find("application/vnd.wap.connectivity-wbxml") >= 0) {
		reader = new OtaReader( OtaReader::Wap );
	    } else if (tmp.find("application/x-wap-prov.browser-settings")
			    >= 0) {
		reader = new OtaReader( OtaReader::Nokia );
	    }

	    if ( reader ) {
		OtaCharacteristicList *list
		    = reader->parseCharacteristics( push.data() );
		QString xml = reader->toXml( push.data() );
		qDebug("%s", xml.latin1());
		delete reader;
		if ( list ) {
		    NetworkInterfaceProperties settings = list->toConfig();
		    applyRemoteSettings( smsMessage.sender(), settings );
		    delete list;
		}
	    }
	}
    }
#endif
}

#ifdef QTOPIA_PHONE
void NetworkSettings::applyRemoteSettings( const QString& from, const NetworkInterfaceProperties& settings )
{
    // Dump the settings to the debug stream.
    settings.debugDump();

    // Ask the user if they wish to apply the settings.
    QString name = settings["name"].toString();

    QString cfgfile;

    QString text;
    if ( name.isEmpty() )
	text = tr("<qt>Received network settings.  Do you wish to apply them?</qt>");
    else {
	for ( QStringList::ConstIterator it = filelist.begin(); it!= filelist.end(); ++it ) {
	    Config cfg(*it,Config::File);
	    cfg.setGroup("Info");
	    if ( cfg.readEntry("Id") == name ) {
		cfgfile = *it;
		break;
	    }
	}
	if ( cfgfile.isEmpty() )
	    text = tr("<qt>Received network settings for \"%1\".  Do you wish to apply them?</qt>").arg(name);
	else
	    text = tr("<qt>Received updated network settings for \"%1\".  Do you wish to apply them?</qt>").arg(name);
    }
    QMessageBox box( tr("Internet"), text, QMessageBox::NoIcon,
		     QMessageBox::Ok | QMessageBox::Default,
		     QMessageBox::Cancel, QMessageBox::NoButton );
    if ( box.exec() != QMessageBox::Ok )
	return;

    // Set a default name based on the sender's phone number if necessary.
    if ( name.isEmpty() ) {
	name = from;
	if ( name.length() > 0 && name[0] == '+' )
	    name = name.mid(1);
    }

    if ( cfgfile.isEmpty() ) {
	cfgfile = addSetting(QPEApplication::qpeDir()+"etc/network/OTAGPRS.conf",FALSE);

	if ( cfgfile.isEmpty() ) {
	    qWarning("Cannot create OTA GPRS network config");
	    return;
	}
    }

    Config cfg(cfgfile,Config::File);
    cfg.setGroup("Info");
    cfg.writeEntry("Id",name);
    cfg.writeEntry("UserSelectable",1);
    QString type = cfg.readEntry("Type");
    NetworkInterface2* plugin = Network::loadPlugin2(type);
    cfg.setGroup("Properties");
    if ( plugin && plugin->setProperties(cfg,settings) )
	return;

    qWarning("Failed to set OTA properties");
}
#endif

