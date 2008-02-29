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


#include "dialup.h"

#include <qtopia/config.h>
#include <qtopia/global.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include "../proxiespage.h"

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qfile.h>
#include <qcstring.h>
#include <qtextstream.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <qlayout.h>

Dialup::Dialup( QWidget* parent ) :
    DialupBase(parent,0,TRUE)
{
    QVBoxLayout *layout = new QVBoxLayout( proxiestab );
    proxies = new ProxiesPage( proxiestab );
    layout->addWidget( proxies );
}

DialupImpl::DialupImpl() : ref(0)
{
}

DialupImpl::~DialupImpl()
{
}

QRESULT DialupImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_Network )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( DialupImpl )
}

static bool setSecret( QFile &paps, const QString& user, const QString& server, const QString& secret, const QString& address)
{
    QString id = user + " " + server + " ";
    QString entry = id + Global::stringQuote(secret);
    if ( !address.isEmpty() )
	entry += " " + address;
    QStringList secrets;
    if ( paps.open(IO_ReadOnly) ) {
	{
	    QTextStream s(&paps);
	    while ( !s.eof() ) {
		QString se = s.readLine();
		if ( se == entry )
		    return TRUE; // already in there
		if ( se.left(id.length()) != id )
		    secrets.append(se);
	    }
	}
	paps.close();
    }
    if ( !secret.isNull() ) // null for deleting secrets
	secrets.append(entry);
    if ( paps.open(IO_WriteOnly) ) {
	fchmod(paps.handle(),0600);
	{
	    QTextStream s(&paps);
	    s << secrets.join("\n") << "\n";
	}
	paps.close();
	return TRUE;
    }
    return FALSE;
}

static bool setPapSecret(const QString& user, const QString& server, const QString& secret, const QString& address)
{
    bool retval = TRUE;
    // joespc    server    "joe's secret" joespc.my.net
    QFile paps("/etc/ppp/pap-secrets" );
    if ( !setSecret( paps, user, server, secret, address ) )
	retval = FALSE;
    QFile chaps( "/etc/ppp/chap-secrets" );
    if ( !setSecret( chaps, user, server, secret, address ) )
	retval = FALSE;
    return retval;
}


bool DialupImpl::doProperties( QWidget *parent, Config& cfg )
{
    Dialup dialog(parent);
    cfg.setGroup("Info");
    QString acname = cfg.readEntry("Name");
    QString acid = cfg.readEntry("Id");
    if ( acid.isNull() ) {
	// Uniqueish ID
	acid = acname;
	acid.replace(QRegExp("[^A-Za-z]"),"");
	acid += QString::number(time(0)^(getpid()<<3));
	cfg.writeEntry("Id",acid);
    }
    dialog.acname->setText(acname);
    cfg.setGroup("Properties");
    bool ro = cfg.readNumEntry("ReadOnly",0);
    QString prev_user = cfg.readEntry("username");
    dialog.username->setText(prev_user);
    dialog.password->setText(cfg.readEntry("password"));
    dialog.phone->setText(cfg.readEntry("phone"));
    dialog.dialmode->setButton(cfg.readNumEntry("dialmode", 0));
    dialog.idletime->setValue(cfg.readNumEntry("idletime",120));
    QString s = cfg.readEntry("atdial","ATDT");
    if ( !s.isEmpty() ) {
	dialog.atdial->insertItem(s);
	dialog.atdial->setCurrentItem(dialog.atdial->count()-1);
    }
    dialog.defaultroute->setChecked(cfg.readBoolEntry("defaultroute",TRUE));
    dialog.usepeerdns->setChecked(cfg.readBoolEntry("usepeerdns",TRUE));
    dialog.gateway->setText(cfg.readEntry("gateway"));
    dialog.dns1->setText(cfg.readEntry("dns1"));
    dialog.dns2->setText(cfg.readEntry("dns2"));
    dialog.gatewaybox->hide(); // UNUSED YET
    dialog.connectdelay->setValue(cfg.readNumEntry("connectdelay",5000)/1000);
    dialog.speed->insertItem(cfg.readEntry("speed","115200"));
    dialog.speed->setCurrentItem(dialog.speed->count()-1);
    dialog.crtscts->setChecked(cfg.readBoolEntry("crtscts",TRUE));

    cfg.setGroup("Proxy");
    dialog.proxies->readConfig( cfg );
    dialog.showMaximized();
    if ( ro || dialog.exec() ) {
	cfg.setGroup("Info");
	cfg.writeEntry("Name", dialog.acname->text());
	cfg.setGroup("Properties");
	cfg.writeEntry("username", dialog.username->text());
	cfg.writeEntry("password", dialog.password->text());
	cfg.writeEntry("phone", dialog.phone->text());
	cfg.writeEntry("dialmode", dialog.dialmode->id(dialog.dialmode->selected()));
	cfg.writeEntry("idletime", dialog.idletime->value());
	cfg.writeEntry("atdial", dialog.atdial->currentText());
	cfg.writeEntry("usepeerdns", dialog.usepeerdns->isChecked());
	cfg.writeEntry("gateway",dialog.gateway->text());
	cfg.writeEntry("dns1",dialog.dns1->text());
	cfg.writeEntry("dns2",dialog.dns2->text());
	cfg.writeEntry("defaultroute", dialog.defaultroute->isChecked());
	cfg.writeEntry("connectdelay", dialog.connectdelay->value()*1000);
	cfg.writeEntry("speed", dialog.speed->currentText());
	cfg.writeEntry("crtscts", dialog.crtscts->isChecked());
	
	cfg.setGroup("Proxy");
	dialog.proxies->writeConfig( cfg );
	cfg.setGroup("Properties");
	mkdir("/etc/ppp/peers",0700);
	QFile options("/etc/ppp/peers/"+acid);
	if ( options.open(IO_WriteOnly) ) {
	    QTextStream opt(&options);
	    opt << dialog.speed->currentText() << "\n";
	    QString dial = cfg.readEntry("atdial","ATDT");
	    dial += cfg.readEntry("phone");
	    //opt << "debug\n";
	    opt << "connect '/usr/sbin/chat " // No tr
		    "-s -v "
		    "ABORT \"NO CARRIER\" " // No tr
		    "ABORT \"NO DIALTONE\" " // No tr
		    "ABORT \"BUSY\" " // No tr
		<< "\"\" ATZ OK " + dial + " CONNECT'\n";
	    if ( dialog.crtscts->isChecked() )
		opt << "crtscts\n";
	    else
		opt << "nocrtscts\n";
	    opt << "noipdefault\n";
	    opt << "modem\n"; // No tr
	    QString user = cfg.readEntry("username");
	    if ( !user.isEmpty() )
		opt << "user " << Global::stringQuote(user) << "\n"; // No tr
	    int dialmode = cfg.readNumEntry("dialmode",0);
	    if ( dialmode == 2 )
		opt << "demand\n"; // No tr
	    int idlet = cfg.readNumEntry("idletime",120);
	    if ( dialmode > 0 && idlet )
		opt << "idle " << idlet << "\n"; // No tr
	    if ( cfg.readBoolEntry("usepeerdns",TRUE) )
		opt << "usepeerdns\n";
	    if ( cfg.readBoolEntry("defaultroute",TRUE) )
		opt << "defaultroute\n";
	    int cdel = cfg.readNumEntry("connectdelay",0);
	    if ( cdel )
		opt << "connect-delay " << cdel << "\n";
	    if ( !acid.isEmpty() )
		opt << "remotename " << acid << "\n";
	    QString pw = cfg.readEntry("password");
	    if ( prev_user != user )
		setPapSecret(prev_user,acid,QString::null,"");
	    if ( !pw.isEmpty() )
		setPapSecret(user,acid,pw,"");
	    else
		setPapSecret(user,acid,QString::null,"");
	    return TRUE;
	}
    }
    return FALSE;
}

QString DialupImpl::dev( Config& cfg ) const
{
    QString d = cfg.readEntry("SerialDevice");
    if ( d.isEmpty() ) {
	QString ct = cardType(cfg);
	if ( !ct.isEmpty() ) {
	    FILE* f = fopen("/var/run/stab", "r");
	    if (!f) f = fopen("/var/state/pcmcia/stab", "r");
	    if (!f) f = fopen("/var/lib/pcmcia/stab", "r");
	    if ( f ) {
		char line[1024];
		char devtype[80];
		char devname[80];
		while ( fgets( line, 1024, f ) ) {
		    // 0       serial  serial_cs       0       ttyS0   4       64
		    if ( sscanf(line,"%*d %s %*s %*s %s", devtype, devname )==2 )
		    {
			if ( ct == devtype ) {
			    fclose(f);
			    return QString("/dev/")+devname;
			}
		    }
		}
		fclose(f);
	    }
	}
    }
    return d;
}

bool DialupImpl::create( Config& )
{
    // nothing special
    return TRUE;
}

bool DialupImpl::remove( Config&cfg )
{
    cfg.setGroup("Info");
    QString peer = cfg.readEntry( "Id" );
    if ( !peer.isEmpty() ) {
	peer = "/etc/ppp/peers/" + peer;
	unlink( peer.latin1() );
    }
    return TRUE;
}

static const char* logfile="/tmp/qpe-pppd-log";

class PppMon : public QObject {
    Q_OBJECT

    enum { Initialize, Connect, Authenticate, Timing, Disappearing } state;
    int age;
    int start;
    bool demand;

public:
    PppMon(bool demanddialing) :
	demand(demanddialing)
    {
	age = 0;
	start = 0;
	state = Initialize;
	progress(demand ? tr("Waiting for activity") : tr("Initializing modem"),1);
	startTimer(demand ? 8000 /* progress less interesting */ : 2000);
    }
    ~PppMon()
    {
    }

protected:
    void timerEvent(QTimerEvent*)
    {
	if ( state == Disappearing ) {
	    killTimers();
	    delete this;
	    return;
	}
	QFile logf(logfile);
	if ( logf.open(IO_ReadOnly) ) {
	    QString log = logf.readAll();
	    log.replace(QRegExp("\r"),"");
	    switch (state) {
	      case Initialize:
		if ( log.find(QRegExp("\nFailed"))>=start ) { // No tr
		    progress(tr("Connection failed"),0);
		    state = Disappearing;
		} else if ( demand ) {
		    int ns;
		    if ( (ns=log.find(QRegExp("Starting link\n")))>=start ) { // No tr
			progress(tr("Connecting"),5);
			state = Connect;
			start = ns+1;
		    }
		} else {
		    int ns;
		    if ( (ns=log.find(QRegExp("\nOK\n -- got it")))>=start ) { // No tr
			progress(tr("Connecting"),1);
			state = Connect;
			start = ns+1;
		    }
		}
		break;
	      case Connect:
		if ( log.find(QRegExp("\nCONNECT[^\n]*\n -- got it"))>=start ) { // No tr
		    progress(tr("Authenticating"),1); // No tr
		    state = Authenticate;
		} else if ( log.find(QRegExp("\nNO DIALTONE"))>=start ) {
		    progress(tr("No dialtone"),0);
		    state = Disappearing;
		} else if ( log.find(QRegExp("\nNO CARRIER"))>=start ) {
		    progress(tr("No carrier"),0);
		    state = Disappearing;
		} else if ( log.find(QRegExp("\nBUSY"))>=start ) {
		    progress(tr("Number is busy"),0);
		    state = Disappearing;
		} else if ( log.find(QRegExp("\nalarm"))>=start ) { // No tr
		    progress(tr("Timed out"),0);
		    state = Disappearing;
		} else if ( log.find(QRegExp("\nFailed"))>=start ) { // No tr
		    progress(tr("Connection failed"),0);
		    state = Disappearing;
		}
		break;
	      case Authenticate:
		if ( log.find(QRegExp("\nPAP authentication failed"))>=start ) { // No tr
		    progress(tr("Authentication Failed"),0);
		    state = Disappearing;
		} else if ( log.find(QRegExp("\nCHAP authentication failed"))>=start ) { // No tr
		    progress(tr("Authentication Failed"),0);
		    state = Disappearing;
		} else if ( log.find(QRegExp("\nipcp: up"))>=start ) { // No tr
		    progress(tr("Connected"),5);
		    if ( demand )
			state = Timing;
		    else
			state = Disappearing;
		}
		break;
	      case Timing:
		if ( log.find(QRegExp("\nConnection terminated"))>=start ) { // No tr
		    progress(tr("Waiting for activity"),1);
		    state = Initialize;
		}
		break;
	      case Disappearing:
		break;
	    }
	    if ( !demand && age++ > 120 ) // fail-safe self destruction
		state = Disappearing;
	} else
	    state = Disappearing;
    }

    void progress(const QString& p, int flag)
    {
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/Network", "progress(QString,int)");
	if ( demand ) flag |= 2;
	e << p << flag;
#endif
    }
};

bool DialupImpl::start( Config& cfg )
{
    return start(cfg,QString::null);
}

bool DialupImpl::start( Config& cfg, const QString& password )
{
    // start pppd
    QString cmd = cfg.readEntry("script");
    if ( cmd.isEmpty() ) {
	if ( !cfg.readBoolEntry("usepeerdns",TRUE) ) {
	    bool any=FALSE;
	    for (int i=1; i<5 && !any; i++)
		any = !cfg.readEntry("dns"+QString::number(i)).isEmpty();
	    if ( any ) {
		QFile rc("/etc/ppp/resolv.conf");
		if ( rc.open(IO_WriteOnly) ) {
		    QTextStream stream(&rc);
		    for (int i=1; i<5; i++) {
			QString dns = cfg.readEntry("dns"+QString::number(i));
			if ( !dns.isEmpty() )
			    stream << "nameserver " << dns << "\n";
		    }
		}
	    }
	}
	QString d = dev(cfg);
	if ( d.isEmpty() ) {
	    qWarning("No device for card type %s",cardType(cfg).latin1());
	    return FALSE;
	}
	cfg.setGroup("Info");
	cmd = "/usr/sbin/pppd updetach " + d + " debug call " + cfg.readEntry("Id"); // No tr
    } else {
	cmd += " " + dev(cfg);
    }
    if ( !password.isNull() )
	cmd += " password " + Global::shellQuote(password); // No tr
    cmd += " >" + QString(logfile) + " 2>&1 &";
    system("cardctl resume");
    system(cmd.latin1());
    cfg.setGroup("Properties");
    int dialmode = cfg.readNumEntry("dialmode",0);
    (void)new PppMon(dialmode==2);
    return TRUE;
}

bool DialupImpl::stop( Config& cfg )
{
    // kill pppd
    QString d = device(cfg);
    if ( !QFile::exists("/var/run/"+d+".pid") )
	d += "0";
    QFile pid("/var/run/"+d+".pid");
    if ( pid.open(IO_ReadOnly) ) {
	QString s(pid.readAll());
	int p = s.toInt();
	if ( p ) {
	    kill(p,SIGTERM); // SIGHUP not enough for demand dialing pppd
	    system("cardctl suspend");
	    return TRUE;
	}
    }
    return FALSE;
}

bool DialupImpl::isAvailable( Config& cfg ) const
{
    QString d = dev(cfg);
    if ( d.isEmpty() )
	return FALSE;
    return QFile::exists(d);
}

bool DialupImpl::needPassword( Config& cfg ) const
{
    return cfg.readEntry("password").isEmpty();
}

#include "dialup.moc"
