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

#define QTOPIA_INTERNAL_LANGLIST
#include <qtopia/network.h>
#include <qtopia/networkinterface.h>
#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include "pluginloader_p.h"

#include <qlistbox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qdict.h>
#include <qtranslator.h>

#include <stdlib.h>

#ifndef QT_NO_COP
class NetworkEmitter : public QCopChannel {
    Q_OBJECT
public:
    NetworkEmitter() : QCopChannel("QPE/Network",qApp)
    {
    }

    void receive(const QCString &msg, const QByteArray&)
    {
	if ( msg == "choicesChanged()" )
	    emit changed();
    }

signals:
    void changed();
};

/*!
  \internal

  Requests that the service \a choice be started. The \a password is
  the password to use if required.
*/
void Network::start(const QString& choice, const QString& password)
{
    QCopEnvelope e("QPE/Network", "start(QString,QString)");
    e << choice << password;
}

/*!
  \class Network network.h
  \brief The Network class provides network access functionality.
  \internal
*/



/*!
  \internal

  Stops the current network service.
*/
void Network::stop()
{
    QCopEnvelope e("QPE/Network", "stop()");
}

static NetworkEmitter *emitter = 0;

/*!
  \internal
*/
void Network::connectChoiceChange(QObject* receiver, const char* slot)
{
    if ( !emitter )
	emitter = new NetworkEmitter;
    QObject::connect(emitter,SIGNAL(changed()),receiver,slot);
}

#endif	// QT_NO_COP
/*!
  \internal
*/
QString Network::settingsDir()
{
    return Global::applicationFileName("Network", "modules"); // No tr
}

#ifndef QT_NO_COP
/*!
  \internal
*/
QStringList Network::choices(QListBox* lb, const QString& dir)
{
    QStringList list;

    if ( lb )
	lb->clear();

    bool userSettings = dir.isEmpty();
    QString adir = dir.isEmpty() ? settingsDir() : dir;
    QDir settingsdir(adir);
    settingsdir.mkdir(adir);
    QStringList invalidConfigs;

    QStringList files = settingsdir.entryList("*.conf");
    for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it ) {
	QString filename = settingsdir.filePath(*it);
	Config cfg(filename, Config::File);
	cfg.setGroup("Info");
	if ( cfg.readNumEntry("UserSelectable",1) ) {
            if (userSettings && !cfg.hasKey("Name") && !cfg.hasKey("Name[]")) {
                //we have a bogus/empty user config file which causes bug 3664
                //ignore it
                invalidConfigs.append(filename); 
                continue;
            }
            QString type = cfg.readEntry("Type");
	    if ( lb )
		lb->insertItem(Resource::loadPixmap("Network/" + 
                            type + '/' + type ),
		    cfg.readEntry("Name"));
	    list.append(filename);
	}
    }

    if (invalidConfigs.count()) {
        for (int i = invalidConfigs.count()-1 ; i >= 0; i--) 
            QFile::remove(invalidConfigs[i]);
    }
    return list;
}

class NetworkServer : public QCopChannel {
    Q_OBJECT
public:
    NetworkServer(QObject* parent) : QCopChannel("QPE/Network",parent), wait(0), reannounce(FALSE)
    {
	firstStart = TRUE;
	up = FALSE;
	QCopChannel* card = new QCopChannel("QPE/Card",parent);
	connect(card,SIGNAL(received(const QCString&,const QByteArray&)),
	    this,SLOT(cardMessage(const QCString&,const QByteArray&)));
	startTimer(2000);
    }

    ~NetworkServer()
    {
	stop();
    }

    bool networkOnline() const
    {
	return up;
    }

private:
    void receive(const QCString &msg, const QByteArray& data)
    {
	if ( msg == "start(QString,QString)" ) {
	    QDataStream stream(data,IO_ReadOnly);
	    QString file,password;
	    stream >> file >> password;
	    if ( file.isEmpty() ) {
		QStringList l = Network::choices();
		for (QStringList::ConstIterator i=l.begin(); i!=l.end(); ++i) {
		    Config cfg(*i,Config::File);
		    cfg.setGroup("Info");
		    QString type = cfg.readEntry("Type");
		    NetworkInterface* plugin = Network::loadPlugin(type);
		    cfg.setGroup("Properties");
		    if ( plugin && plugin->isAvailable(cfg) ) {
			file = *i;
			break;
		    }
		}
		if ( file.isEmpty() ) {
		    QCopEnvelope("QPE/Network", "failed()");
		    return;
		}
	    }
	    start(file,password);
	} else if ( msg == "stop()" ) {
	    stop();
	} else if ( msg == "choicesChanged()" ) {
	    examineNetworks();
	} else if ( msg == "announceChoices()" ) {
	    reannounce = TRUE;
	    examineNetworks();
	}
    }

private slots:
    void cardMessage(const QCString &msg, const QByteArray&)
    {
	if ( msg == "stabChanged()" )
	    examineNetworks();
    }

private:
    void examineNetworks()
    {
	QStringList l = Network::choices();
	bool wasup = up; up=FALSE;
	QStringList pavailable = available;
	available.clear();
	for (QStringList::ConstIterator it=l.begin(); it!=l.end(); ++it) {
	    Config cfg(*it,Config::File);
	    cfg.setGroup("Info");
	    QString type = cfg.readEntry("Type");
	    NetworkInterface* plugin = Network::loadPlugin(type);
//#define DEBUG_IGNORE_LAN
#ifdef DEBUG_IGNORE_LAN
	    // Qtopia doesn't currently understand multiple
	    // services being active simultaneously (and doesn't
	    // support the routing control needed to make that useful).
	    // For debugging purposes, we can hide the LAN connection
	    // (eg. NFS mounted development system), so that the other
	    // services can be started.
	    if ( type == "lan" )
		plugin = 0;
#endif
	    cfg.setGroup("Properties");
	    if ( plugin ) {
		if ( plugin->isActive(cfg) ) {
		    up = TRUE;
		    if ( firstStart )
			Network::start(*it);
		} else if ( cfg.readNumEntry("Auto",0)
			    && plugin->isAvailable(cfg) )
		{
		    if ( firstStart && wait%128==0 )
			Network::start(*it);
		}
		if ( plugin->isAvailable(cfg) )
		    available.append(*it);
	    }
	}

	// Try to work around unreproducible bug whereby
	// the netmon applet shows wrong state.
	bool reannounceWorkaround = wait<0;

	if ( available != pavailable || reannounce || reannounceWorkaround ) {
	    QCopEnvelope e("QPE/Network", "available(QStringList)");
	    e << available;
	}
	if ( up != wasup || reannounce || reannounceWorkaround ) {
	    QCopEnvelope("QPE/Network", up ? "up()" : "down()");
	}

	reannounce = FALSE;
	firstStart = FALSE;
    }

    void start( const QString& file, const QString& password )
    {
	if ( !current.isEmpty() )
	    stop();
	current = QString::null;
	Config cfg(file, Config::File);
	cfg.setGroup("Info");
	QString type = cfg.readEntry("Type");
	NetworkInterface* plugin = Network::loadPlugin(type);
	bool started = FALSE;
	if ( plugin ) {
	    cfg.setGroup("Properties");
	    if ( plugin->start(cfg,password) ) {
		Network::writeProxySettings( cfg );
		current = file;
		wait=0;
		startTimer(400);
		started = TRUE;
	    }
	}
	if ( !started ) {
	    QCopEnvelope("QPE/Network", "failed()");
	}
    }

    void stop()
    {
	bool stopped = FALSE;
	if ( !current.isEmpty() ) {
	    Config cfg(current, Config::File);
	    cfg.setGroup("Info");
	    QString type = cfg.readEntry("Type");
	    NetworkInterface* plugin = Network::loadPlugin(type);
	    if ( plugin ) {
		cfg.setGroup("Properties");
		if ( plugin->stop(cfg) ) {
		    current = QString::null;
		    wait=0;
		    startTimer(400);
		    stopped = TRUE;
		}
	    }
	}
	if ( !stopped ) {
	    QCopEnvelope("QPE/Network", "failed()");
	}
    }

    void timerEvent(QTimerEvent*)
    {
	examineNetworks();
	if ( wait >= 0 ) {
	    if ( up == !current.isNull() ) {
		// done
		killTimers();
		if ( up ) {
		    startTimer(3000); // monitor link
		    wait = -1;
		}
	    } else {
		wait++;
		if ( wait == 600 && !current.isNull()) {
		    killTimers(); // forget about it after 240 s
		    QCopEnvelope("QPE/Network", "failed()");
		    up = FALSE;
		}
	    }
	} else if ( !up ) {
	    killTimers();
	}
    }

private:
    QStringList available;
    QString current;
    bool up;
    int wait;
    bool reannounce;
    bool firstStart;
};

static NetworkServer* ns=0;

/*!
  \internal
*/
QString Network::serviceName(const QString& service)
{
    Config cfg(service, Config::File);
    cfg.setGroup("Info");
    return cfg.readEntry("Name");
}

/*!
  \internal
*/
QString Network::serviceType(const QString& service)
{
    Config cfg(service, Config::File);
    cfg.setGroup("Info");
    return cfg.readEntry("Type");
}

/*!
  \internal
*/
bool Network::serviceNeedsPassword(const QString& service)
{
    Config cfg(service,Config::File);
    cfg.setGroup("Info");
    QString type = cfg.readEntry("Type");
    NetworkInterface* plugin = Network::loadPlugin(type);
    cfg.setGroup("Properties");
    return plugin ? plugin->needPassword(cfg) : FALSE;
}
#endif // QT_NO_COP
/*!
  \internal
*/
bool Network::networkOnline()
{
#ifndef QT_NO_COP
    return ns && ns->networkOnline();
#else
    return FALSE;
#endif
}

// copy the proxy settings of the active config over to the Proxies.conf file
/*!
  \internal
*/
void Network::writeProxySettings( Config &cfg )
{
    Config proxy( Network::settingsDir() + "/Proxies.conf", Config::File );
    proxy.setGroup("Info");
    proxy.writeEntry("UserSelectable",0);
    proxy.setGroup("Properties");
    cfg.setGroup("Proxy");
    proxy.writeEntry("type", cfg.readEntry("type") );
    proxy.writeEntry("autoconfig", cfg.readEntry("autoconfig") );
    proxy.writeEntry("httphost", cfg.readEntry("httphost") );
    proxy.writeEntry("httpport", cfg.readEntry("httpport") );
    proxy.writeEntry("ftphost", cfg.readEntry("ftphost") );
    proxy.writeEntry("ftpport", cfg.readEntry("ftpport") );
    proxy.writeEntry("noproxies", cfg.readEntry("noproxies") );
    cfg.setGroup("Properties");
}

#ifndef QT_NO_COP
/*!
  \internal
*/
void Network::createServer(QObject* parent)
{
    ns = new NetworkServer(parent);
}

/*!
  \internal
*/
int Network::addStateWidgets(QWidget* parent)
{
    int n=0;
    QStringList l = Network::choices();
    QVBoxLayout* vb = new QVBoxLayout(parent);
    for (QStringList::ConstIterator it=l.begin(); it!=l.end(); ++it) {
	Config cfg(*it,Config::File);
	cfg.setGroup("Info");
	QString type = cfg.readEntry("Type");
	NetworkInterface* plugin = Network::loadPlugin(type);
	cfg.setGroup("Properties");
	if ( plugin ) {
	    QWidget* w;
	    if ( (w=plugin->addStateWidget(parent,cfg)) ) {
		n++;
		vb->addWidget(w);
	    }
	}
    }
    return n;
}

static QDict<NetworkInterface> *ifaces = 0;
static PluginLoaderIntern *loader = 0;

/*!
  \internal
*/
NetworkInterface* Network::loadPlugin(const QString& type)
{
    if ( type.isEmpty() )
	return 0;
#ifndef QT_NO_COMPONENT
    if ( !ifaces ) ifaces = new QDict<NetworkInterface>;
    if ( !loader ) loader = new PluginLoaderIntern( "network" ); // No tr
    NetworkInterface *iface = ifaces->find(type);
    if ( !iface ) {
	if ( loader->queryInterface( type, IID_Network, (QUnknownInterface**)&iface ) != QS_OK )
	    return 0;
	ifaces->insert(type,iface);
    }
    return iface;
#else
    return 0;
#endif
}

NetworkInterface2* Network::loadPlugin2(const QString& type)
{
    if ( type.isEmpty() )
	return 0;
#ifndef QT_NO_COMPONENT
    if ( !ifaces )
	ifaces = new QDict<NetworkInterface>;
    if ( !loader )
	loader = new PluginLoaderIntern( "network" ); // No tr
    NetworkInterface *iface = ifaces->find(type);
    NetworkInterface2 *iface2 = 0;
    if ( !iface ) {
	if ( loader->queryInterface( type, IID_Network2, (QUnknownInterface**)&iface2 ) != QS_OK )
	    return 0;
	ifaces->insert(type,iface2);
	return iface2;
    }
    if ( iface->queryInterface(IID_Network2, (QUnknownInterface**)&iface2) != QS_OK )
	return 0;
    return iface2;
#else
    return 0;
#endif
}

#include "network.moc"
#endif	// QT_NO_COP
