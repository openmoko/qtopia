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

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/network.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/global.h>
#include <qtopia/process.h>
#include "../proxiespage.h"

#include <qbuttongroup.h>
#include <qvbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qtabwidget.h>
#include <qbuffer.h>
#include <qspinbox.h>
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

    static void changeLanScheme(const QString& lan_or_scheme)
    {
#ifdef USE_SCHEMES	
	Config c("Network");
	c.setGroup("Lan");
	QString s = toSchemeId(lan_or_scheme);
	c.writeEntry("Scheme",s);
	system("cardctl scheme " + s);
#else	
	QStringList srv = Network::choices();
	for (QStringList::ConstIterator it=srv.begin(); it!=srv.end(); ++it) {
	    QString t = Network::serviceType(*it);
	    if ( t == "lan" ) {
		Config cfg(*it,Config::File);
		cfg.setGroup("Info");
		QString n = cfg.readEntry("Name");
		if ( n == lan_or_scheme ) {
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
    void changeLanSchemeSlot(const QString& lan_or_scheme)
    {
	changeLanScheme(lan_or_scheme);
    }
};

class WepKeyValidator : public QValidator {
public:
    WepKeyValidator(QWidget* parent) :
	QValidator(parent)
    {
    }

    State validate( QString& key, int& curs ) const
    {
	QString k;
	int hexes=0;
	int ncurs=0;
	for (int i=0; i<(int)key.length(); i++) {
	    char c=key[i].lower().latin1();
	    if ( c>='0' && c<='9' || c>='a' && c<='z' ) {
		if ( hexes == 2 ) {
		    hexes = 0;
		    k += ':';
		    if ( i<curs ) ncurs++;
		}
		k += c;
		hexes++;
		if ( i<curs ) ncurs++;
	    } else if ( c == ':' && hexes==2 ) {
		hexes = 0;
		k += c;
		if ( i<curs ) ncurs++;
	    }
	}
	key = k;
	curs = ncurs;
	return Acceptable;
    }

    //void fixup( QString& key ) const
    //{
    //}
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

    connect(wep_passphrase, SIGNAL(textChanged(const QString&)), this, SLOT(chooseDefaultWepKey()));
    connect(wep_key0, SIGNAL(textChanged(const QString&)), this, SLOT(chooseDefaultWepKey()));
    connect(wep_key1, SIGNAL(textChanged(const QString&)), this, SLOT(chooseDefaultWepKey()));
    connect(wep_key2, SIGNAL(textChanged(const QString&)), this, SLOT(chooseDefaultWepKey()));
    connect(wep_key3, SIGNAL(textChanged(const QString&)), this, SLOT(chooseDefaultWepKey()));
    connect(wep_type, SIGNAL(activated(int)), this, SLOT(wepTypeChanged(int)));
    QValidator* key = new WepKeyValidator(this);
    wep_key0->setValidator(key);
    wep_key1->setValidator(key);
    wep_key2->setValidator(key);
    wep_key3->setValidator(key);

    readConfig();
}

void Lan::wepTypeChanged(int i)
{
    wep_choice->setEnabled(i>0);
}

void Lan::chooseDefaultWepKey()
{
    if ( sender() == wep_passphrase )
	wep_passphrase_on->setChecked(TRUE);
    else if ( sender() == wep_key0 )
	wep_key0_on->setChecked(TRUE);
    else if ( sender() == wep_key1 )
	wep_key1_on->setChecked(TRUE);
    else if ( sender() == wep_key2 )
	wep_key2_on->setChecked(TRUE);
    else if ( sender() == wep_key3 )
	wep_key3_on->setChecked(TRUE);
}

void Lan::ipSelect( int id )
{
    tabs->setTabEnabled( tcpip, id==0 );
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

    QString ssid = config.readEntry("SSID");
    if ( !ssid.isEmpty() ) {
	wlan_ssid->insertItem(ssid);
	wlan_ssid->setCurrentItem(wlan_ssid->count()-1);
    }
    bool ah=config.readEntry("IS_ADHOC").lower()=="y";
    (ah ? wlan_adhoc : wlan_infrastructure)->setChecked(TRUE);
    wlan_channel->setValue(config.readNumEntry("CHANNEL"));
    wep_passphrase->setText(config.readEntry("PRIV_GENSTR"));
    wep_key0->setText(config.readEntry("dot11WEPDefaultKey0"));
    wep_key1->setText(config.readEntry("dot11WEPDefaultKey1"));
    wep_key2->setText(config.readEntry("dot11WEPDefaultKey2"));
    wep_key3->setText(config.readEntry("dot11WEPDefaultKey3"));
    int weptype = config.readEntry("PRIV_KEY128")=="true" ? 2 : 1;
    QString wep = config.readEntry("WEP");
    if ( wep == "PP" )
	wep_passphrase_on->setChecked(TRUE);
    else if ( wep == "K0" )
	wep_key0_on->setChecked(TRUE);
    else if ( wep == "K1" )
	wep_key1_on->setChecked(TRUE);
    else if ( wep == "K2" )
	wep_key2_on->setChecked(TRUE);
    else if ( wep == "K3" )
	wep_key3_on->setChecked(TRUE);
    else
	weptype=0;
    wep_type->setCurrentItem(weptype);
    wepTypeChanged(weptype);

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
	    if ( (n == nm
#ifdef USE_SCHEMES
		|| toSchemeId(n) == toSchemeId(nm)
#endif
		) && cfg != config
	    ) {
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

	config.writeEntry("WLAN_ENABLE","y");
	config.writeEntry("SSID",wlan_ssid->currentText());
	config.writeEntry("IS_ADHOC",wlan_adhoc->isChecked() ? "y" : "n");
	config.writeEntry("PRIV_KEY128",wep_type->currentItem()==2 ? "true" : "false");
	config.writeEntry("CHANNEL",wlan_channel->value());
	config.writeEntry("PRIV_GENSTR", wep_passphrase->text());
	config.writeEntry("dot11PrivacyInvoked", wep_type->currentItem()==0 ? "false" : "true");
	config.writeEntry("AuthType", wep_type->currentItem()==0 ? "opensystem" : "sharedkey");
	int defkey = wep_choice->id(wep_choice->selected());
	if ( defkey < 1 || defkey > 4 )
	    defkey = 1;
	config.writeEntry("dot11WEPDefaultKeyID",defkey-1);
	config.writeEntry("dot11WEPDefaultKey0", wep_key0->text());
	config.writeEntry("dot11WEPDefaultKey1", wep_key1->text());
	config.writeEntry("dot11WEPDefaultKey2", wep_key2->text());
	config.writeEntry("dot11WEPDefaultKey3", wep_key3->text());
	QString wep;
	if ( wep_passphrase_on->isChecked() )
	    wep = "PP";
	else if ( wep_key0_on->isChecked() )
	    wep = "K0";
	else if ( wep_key1_on->isChecked() )
	    wep = "K1";
	else if ( wep_key2_on->isChecked() )
	    wep = "K2";
	else if ( wep_key3_on->isChecked() )
	    wep = "K3";
	else
	    wep = "NO";
	config.writeEntry("WEP",wep);

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
    // PCMCIA card will have "CardType = network" entry
    config.setGroup("Properties");
    QString cardType = config.readEntry("CardType");

    if (cardType == "network") // No tr
	return writePcmciaNetworkOpts(config, scheme);
    else
	return writeBuiltinNetworkOpts(config, scheme);
}

bool Lan::writeBuiltinNetworkOpts( Config &config, QString scheme )
{
    // Since no standard exists for configuring networks, this
    // function must be implemented by the system integrator.
    //
    // The procedure is:
    //  1. write the network configuration file to a tmp file.
    //  2. stop the network.
    //  3. move the tmp file to the network config file.
    //  4. start the network.

    return FALSE;
}

bool Lan::writePcmciaNetworkOpts( Config &config, QString scheme )
{
    QString prev = "/etc/pcmcia/network.opts";
    QFile prevFile(prev);
    if ( !prevFile.open( IO_ReadOnly ) )
	return FALSE;

    QString tmp = prev + "-qpe-new";
    QFile tmpFile(tmp);
    if ( !tmpFile.open( IO_WriteOnly ) )
	return FALSE;

    bool retval = TRUE;
    
    QTextStream in( &prevFile );
    QTextStream out( &tmpFile );

    config.setGroup("Info");
    QString nm = config.readEntry( "Name" );
    
    config.setGroup("Properties");

    //For DHCP to work, we have to remove the TCP/IP fields
    bool dhcp = config.readEntry("DHCP","y") != "n";
    
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
			if ( eq > 0 ) {
			    if ( dhcp && k.left(4) != "DHCP" )
				v = "";
			    else
				v = Global::shellQuote(v);
			} else {
			    if ( v.isEmpty() )
				v = "{ return; }"; // No tr
			}
			line = "    " + k + s + v;
		    }
		}
	    } else {
		if ( wline.left(scheme.length()+7) == scheme + ",*,*,*)" ) {
		    found=TRUE;
		} else if ( wline == "esac" || wline == "*,*,*,*)" ) {
		    // end - add new entry
		    // Not all fields have a GUI, but all are supported
		    // in the network configuration files.
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
			out << "    " << *f << "=" 
			    << Global::shellQuote(dhcp ? QString("") : config.readEntry(*f,""))
			    << "\n";
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

    QString prev2 = "/etc/pcmcia/wlan-ng.opts";
    prevFile.setName(prev2);
    if ( !prevFile.open( IO_ReadOnly ) )
	return FALSE;

    QString tmp2 = prev2 + "-qpe-new";
    tmpFile.setName(tmp2);
    if ( !tmpFile.open( IO_WriteOnly ) )
	return FALSE;

    QTextStream win( &prevFile );
    QTextStream wout( &tmpFile );

    found=FALSE;
    done=FALSE;
    QString wep = config.readEntry("WEP");
    while ( !win.atEnd() ) {
	QString line = win.readLine();
	QString wline = line.simplifyWhiteSpace();
	if ( !done ) {
	    if ( found ) {
		if ( wline == ";;" ) {
		    done = TRUE;
		} else {
		    int eq=wline.find("=");
		    QString k,v;
		    if ( eq > 0 ) {
			k = wline.left(eq);
		    }
		    if ( !k.isNull() ) {
			QString v = config.readEntry(k);
			line = "    " + k + "=" + Global::shellQuote(v);
		    }
		}
	    } else {
		if ( wline.left(scheme.length()+7) == scheme + ",*,*,*)" ) {
		    found=TRUE;
		} else if ( wline == "esac" || wline == "*,*,*,*)" ) {
		    // end - add new entry
		    // Not all fields have a GUI, but all are supported
		    // in the wlan configuration files.
		    static const char* txtfields[] = {
			"WLAN_ENABLE",
			"USER_MIBS", "dot11ExcludeUnencrypted", "PRIV_GENERATOR",
			"PRIV_KEY128", "BCNINT", "BASICRATES", "OPRATES",
			"SSID", "IS_ADHOC", "CHANNEL", "PRIV_GENSTR",
			"dot11WEPDefaultKey0", "dot11WEPDefaultKey1",
			"dot11WEPDefaultKey2", "dot11WEPDefaultKey3",
			"AuthType", "dot11PrivacyInvoked", "dot11WEPDefaultKeyID",
			0
		    };
		    wout << scheme << ",*,*,*)" << "\n";
		    const char** f = txtfields;
		    while (*f) {
			wout << "    " << *f << "=" 
			    << Global::shellQuote(config.readEntry(*f,""))
			    << "\n";
			++f;
		    }
		    wout << "    ;;\n";
		    done = TRUE;
		}
	    }
	}
	wout << line << "\n";
    }

    prevFile.close();
    tmpFile.close();

    //system("cardctl suspend");
    system("cardctl eject");

    if ( system( "mv " + tmp + " " + prev ) )
	retval = FALSE;
    if ( system( "mv " + tmp2 + " " + prev2 ) )
	retval = FALSE;
#ifdef USE_SCHEMES
    if ( retval )
	SchemeChanger::changeLanScheme(scheme);
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
	bool found=FALSE;
	for (QStringList::ConstIterator it=lans.begin(); !found && it!=lans.end(); ++it) {
	    if ( toSchemeId(*it) == scheme )
		found = TRUE;
	}
	if ( !found )
	    SchemeChanger::changeLanScheme( lans[0] );
    } else {
	{
	    Config c("Network");
	    c.setGroup("Lan");
	    c.writeEntry("Scheme","default"); // No tr
	}
	SchemeChanger::changeLanScheme( "default" ); // No tr
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

bool LanImpl::create( Config& cfg )
{
    QStringList lans = findLans();
    QString nm = cfg.readEntry("Name");
    int n=1;
    while ( lans.contains(nm) ) {
	nm = "LAN " + QString::number(++n);
    }
    cfg.writeEntry("Name",nm);
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

class LanStateImpl : public LanState {
    Q_OBJECT
public:
    LanStateImpl(QWidget* parent, const QString &dev) : LanState(parent), device(dev)
    {
	QStringList lans = findLans();
	QString scheme = findScheme(lans);
	if ( lans.count() > 1 ) {
	    services->insertStringList(lans);
	    services->setCurrentItem(lans.findIndex(scheme));
	    SchemeChanger* sc = new SchemeChanger(this);
	    QObject::connect(services,SIGNAL(activated(const QString&)),
		sc,SLOT(changeLanSchemeSlot(const QString&)));
	} else {
	    services->hide();
	    services_label->hide();
	}
	if ( !device[device.length()-1].isDigit() )
	    device += "0";
	tid = startTimer(1000);
	updateDisplay();
    }

    void timerEvent(QTimerEvent*)
    {
	updateDisplay();
	if ( ipaddress->text()[0] != 'e' ) {
	    // slow down
	    killTimer(tid);
	    tid = startTimer(5000);
	}
    }

    bool ok() const
    {
	return !ipaddress->text().isEmpty();
    }

private slots:
    void updateDisplay()
    {
	QString ipaddr;
	Process p(QStringList() << "ifconfig" << device);
	if ( p.exec( "", ipaddr ) ) {
	    ipaddr.replace(QRegExp(".*inet addr:"),"");
	    ipaddr.replace(QRegExp(" .*"),"");
	    ipaddress->setText(ipaddr);
	} else {
	    ipaddress->setText("");
	}
    }

private:
    int tid;
    QString device;
};

QWidget* LanImpl::addStateWidget( QWidget* parent, Config& cfg ) const
{
    if ( isActive(cfg) ) {
	cfg.setGroup( "Properties" );
	QString dev = cfg.readEntry( "Device" );
	LanStateImpl* state = new LanStateImpl(parent, dev);
	if ( state->ok() )
	    return state;
	delete state;
    }
    return 0;
}

bool LanImpl::isActive( Config& cfg ) const
{
    if ( NetworkInterface::isActive(cfg) ) {
	QString n,s;
	{
	    cfg.setGroup("Info");
	    n = toSchemeId(cfg.readEntry("Name"));
	    Config c("Network");
	    c.setGroup("Lan");
	    s = c.readEntry("Scheme");
	}
	if ( s.isNull() ) {
	    s=n;
	    SchemeChanger::changeLanScheme( s );
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
