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
#include <qpe/config.h>
#include <qpe/network.h>
#ifdef QWS
#include <qpe/qcopenvelope_qws.h>
#endif
#include <qpe/global.h>
#include <qpe/process.h>
#include "../proxiespage.h"

#include <qbuttongroup.h>
#include <qvbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qtabwidget.h>
#include <qbuffer.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include "lan.h"
#include "lanstate.h"
#include <qlayout.h>

#include <stdlib.h>

#define USE_SCHEMES

#ifdef USE_SCHEMES
static QString toSchemeId(const QString& s)
{
    QString r;
    for (int i=0; i<(int)s.length(); i++) {
	char c = s[i].lower().latin1();
	if ( c>='a' && c<='z' || i>0 && c>='0' && c<='9' )
	    r += s[i];
	//else
	    //r += "_U" + QString::number(s[i].unicode()) + "_";
    }
    return r;
}
#endif

class SchemeChanger : public QObject {
    Q_OBJECT
public:
    SchemeChanger(QObject* parent) :
	QObject(parent)
    {
    }

    static void changeScheme(const QString& sc)
    {
#ifdef USE_SCHEMES	
	Config c("Network");
	c.setGroup("Lan");
	c.writeEntry("Scheme",sc);
	QString s = toSchemeId(sc);
	system("cardctl scheme " + s);
#else	
	QStringList srv = Network::choices();
	for (QStringList::ConstIterator it=srv.begin(); it!=srv.end(); ++it) {
	    QString t = Network::serviceType(*it);
	    if ( t == "lan" ) {
		Config cfg(*it,Config::File);
		cfg.setGroup("Info");
		QString n = cfg.readEntry("Name");
		if ( n == sc ) {
		    Lan::writeNetworkOpts( cfg );
		}
	    }
	}	
#endif	
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/Network","choicesChanged()");
#endif
    }

public slots:
    void changeSchemeSlot(const QString& sc) { changeScheme(sc); }
};



Lan::Lan(Config& cfg, QWidget* parent) :
    LanBase(parent,0,TRUE),
    config(cfg)
{
    ipGroup = new QButtonGroup( this );
    ipGroup->hide();
    ipGroup->insert( manualIP );
    ipGroup->insert( autoIP );
    connect( ipGroup, SIGNAL(clicked(int)), this, SLOT(ipSelect(int)) );
    ipGroup->setButton( 1 );

    QBoxLayout *proxyLayout  = new QVBoxLayout( tab_2 ); 
    proxies = new ProxiesPage( tab_2 );
    proxyLayout->addWidget( proxies );
    
    readConfig();
}

void Lan::ipSelect( int id )
{
    tabs->setTabEnabled( tcpip, id==0 );
    tabs->setTabEnabled( dns, id==0 );
}

void Lan::readConfig()
{
    int id = config.readEntry("DHCP","y") != "n";
    ipGroup->setButton( id );
    ipSelect( id );

    config.setGroup("Info");
    acname->setText(config.readEntry("Name","LAN"));

    config.setGroup("Properties");
    ip->setText( config.readEntry("IPADDR") );
    subnet->setText( config.readEntry("NETMASK","255.255.255.0") );
    broadcast->setText( config.readEntry("BROADCAST","") );
    gateway->setText( config.readEntry("GATEWAY") );
    dns1->setText( config.readEntry("DNS_1") );
    dns2->setText( config.readEntry("DNS_2") );

    config.setGroup("Proxy");
    proxies->readConfig( config );
}

bool Lan::writeConfig()
{
    QString nm = acname->text();

    // make sure we don't duplicate the name
    QStringList srv = Network::choices();
    for (QStringList::ConstIterator it=srv.begin(); it!=srv.end(); ++it) {
	QString t = Network::serviceType(*it);
	if ( t == "lan" ) {
	    Config cfg(*it,Config::File);
	    cfg.setGroup("Info");
	    QString n = cfg.readEntry("Name");
	    if ( n == nm && cfg != config ) {
		QMessageBox::warning(0, tr("LAN Setup"), 
				     tr( "This name already\nexists. Please choose a\ndifferent name.") );
		return FALSE;
	    }
	}
    }

#ifdef USE_SCHEMES    
    QString scheme;
#endif
    {
	config.setGroup("Info");
	config.writeEntry("Name",nm);
#ifdef USE_SCHEMES    
	scheme = toSchemeId(nm);
	if ( scheme.isEmpty() )
	    scheme = "*"; // The default config
#endif    
	config.setGroup("Properties");

	config.writeEntry("DHCP",autoIP->isChecked() ? "y" : "n");
	config.writeEntry("IPADDR",ip->text());
	config.writeEntry("NETMASK",subnet->text());
	config.writeEntry("BROADCAST",broadcast->text());
	config.writeEntry("GATEWAY",gateway->text());
	config.writeEntry("DNS_1",dns1->text());
	config.writeEntry("DNS_2",dns2->text());

	config.setGroup("Proxy");
	proxies->writeConfig( config );
    }

#ifdef USE_SCHEMES    
    return writeNetworkOpts( config, scheme );
#else
    return TRUE;
#endif    
}

bool Lan::writeNetworkOpts( Config &config, QString scheme )
{
    QString prev = "/etc/pcmcia/network.opts";
    QFile prevFile(prev);
    if ( !prevFile.open( IO_ReadOnly ) )
	return FALSE;

    QString tmp = "/etc/pcmcia/network.opts-qpe-new";
    QFile tmpFile(tmp);
    if ( !tmpFile.open( IO_WriteOnly ) )
	return FALSE;

    bool retval = TRUE;
    
    QTextStream in( &prevFile );
    QTextStream out( &tmpFile );

    config.setGroup("Info");
    QString nm = config.readEntry( "Name" );
    
    config.setGroup("Properties");
    
    QString line;
    bool found=FALSE;
    bool done=FALSE;
    while ( !in.atEnd() ) {
	QString line = in.readLine();
	QString wline = line.simplifyWhiteSpace();
	if ( !done ) {
	    if ( found ) {
		if ( wline == ";;" ) {
		    done = TRUE;
		} else {
		    int eq=wline.find("=");
		    QString k,s,v;
		    if ( eq > 0 ) {
			k = wline.left(eq);
			s = "=";
		    } else if ( wline.left(8) == "start_fn" ) {
			k = wline.left(8);
			s = " () ";
		    } else if ( wline.left(7) == "stop_fn" ) {
			k = wline.left(7);
			s = " () ";
		    }
		    if ( !k.isNull() ) {
			QString v = config.readEntry(k);
			line = "    " + k + s + v;
		    }
		}
	    } else {
		if ( wline.left(scheme.length()+7) == scheme + ",*,*,*)" ) {
		    found=TRUE;
		} else if ( wline == "esac" || wline == "*,*,*,*)" ) {
		    // end - add new entry
		    // Not all fields have a GUI, but all are supported
		    // in the letwork configuration files.
		    static const char* txtfields[] = {
			"IF_PORT", "DHCP_HOSTNAME", "NETWORK",
			"DOMAIN", "SEARCH", "MOUNTS",
			"MTU", "NO_CHECK", "NO_FUSER",
			"IPADDR", "NETMASK", "BROADCAST", "GATEWAY",
			"DNS_1", "DNS_2", "DNS_3", 0
		    };
		    out << scheme << ",*,*,*)" << "\n"
			<< "    INFO=" << Global::shellQuote(nm) << "\n"
			<< "    BOOTP=" << config.readEntry("BOOTP","n") << "\n"
			<< "    DHCP=" << config.readEntry("DHCP","y") << "\n"
			<< "    start_fn () " << config.readEntry("start_fn","{ return; }") << "\n"
			<< "    stop_fn () " << config.readEntry("stop_fn","{ return; }") << "\n"
			;
		    const char** f = txtfields;
		    while (*f) {
			out << "    " << *f << "=" << config.readEntry(*f,"") << "\n";
			++f;
		    }
		    out << "    ;;\n";
		    done = TRUE;
		}
	    }
	}
	out << line << "\n";
    }

    prevFile.close();
    tmpFile.close();

    //system("cardctl suspend");
    system("cardctl eject");

    if ( system( "mv " + tmp + " " + prev ) )
	retval = FALSE;
#ifdef USE_SCHEMES
    if ( retval )
	SchemeChanger::changeScheme(scheme);
#endif

    //system("cardctl resume");
    system("cardctl insert");

    return retval;
}

void Lan::accept()
{
    if ( writeConfig() )
	QDialog::accept();
}

static QStringList findLans(const QString& skip=QString::null)
{
    QStringList srv = Network::choices();
    QStringList lans;
    for (QStringList::ConstIterator it=srv.begin(); it!=srv.end(); ++it) {
	QString t = Network::serviceType(*it);
	if ( t == "lan" ) {
	    Config cfg(*it,Config::File);
	    cfg.setGroup("Info");
	    QString n = cfg.readEntry("Name");
	    if ( n != skip )
		lans.append(n);
	}
    }
    return lans;
}

static QString findScheme(const QStringList& lans)
{
    QString scheme;
    if ( lans.count() > 0 ) {
	{
	    Config c("Network");
	    c.setGroup("Lan");
	    scheme = c.readEntry("Scheme");
	}
	if ( !lans.contains(scheme) )
	    SchemeChanger::changeScheme( lans[0] );
    } else {
	{
	    Config c("Network");
	    c.setGroup("Lan");
	    c.writeEntry("Scheme","default"); // No tr
	}
	SchemeChanger::changeScheme( "default" );
    }
    return scheme;
}


LanImpl::LanImpl()
{
}

LanImpl::~LanImpl()
{
}

QRESULT LanImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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
    Q_CREATE_INSTANCE( LanImpl )
}

bool LanImpl::doProperties( QWidget *parent, Config& cfg )
{
    Lan dialog(cfg,parent);
    dialog.showMaximized();
    return dialog.exec();
}

bool LanImpl::create( Config& )
{
    // nothing special
    return TRUE;
}

bool LanImpl::remove( Config& cfg )
{
#ifdef USE_SCHEMES
    // Switch to another
    cfg.setGroup("Info");
    QStringList lans = findLans(cfg.readEntry("Name"));
    findScheme(lans);
#endif
    return TRUE;
}

bool LanImpl::start( Config& cfg )
{
    cfg.setGroup("Info");
#ifdef USE_SCHEMES
    QString scheme = toSchemeId(cfg.readEntry("Name"));
    system( "cardctl scheme " + scheme );
    return TRUE;
#else
    return Lan::writeNetworkOpts( cfg );
#endif
}

bool LanImpl::stop( Config& cfg )
{
    // Not implemented
    return !isActive(cfg);
}

QWidget* LanImpl::addStateWidget( QWidget* parent, Config& cfg ) const
{
    QStringList lans = findLans();
    QString scheme = findScheme(lans);

    if ( !isActive(cfg) )
	return 0;

    QString ipaddr;
    Process p(QStringList() << "ifconfig" << "eth0");
    if ( p.exec( "", ipaddr ) ) {
	ipaddr.replace(QRegExp(".*inet addr:"),"");
	ipaddr.replace(QRegExp(" .*"),"");
	LanState* state = new LanState(parent);
	if ( lans.count() > 1 ) {
	    state->services->insertStringList(lans);
	    state->services->setCurrentItem(lans.findIndex(scheme));
	    SchemeChanger* sc = new SchemeChanger(state);
	    QObject::connect(state->services,SIGNAL(activated(const QString&)),
		sc,SLOT(changeSchemeSlot(const QString&)));
	} else {
	    state->services->hide();
	    state->services_label->hide();
	}
	state->ipaddress->setText(ipaddr);
	return state;
    }
    return 0;
}

bool LanImpl::isActive( Config& cfg ) const
{
    if ( NetworkInterface::isActive(cfg) ) {
	QString n,s;
	{
	    cfg.setGroup("Info");
	    n = cfg.readEntry("Name");
	    Config c("Network");
	    c.setGroup("Lan");
	    s = c.readEntry("Scheme");
	}
	if ( s.isNull() ) {
	    s=n;
	    SchemeChanger::changeScheme( s );
	}
	if ( s==n )
	    return TRUE;
    }
    return FALSE;
}

bool LanImpl::isAvailable( Config& cfg ) const
{
    return NetworkInterface::isAvailable(cfg);
}


#include "lan.moc"
