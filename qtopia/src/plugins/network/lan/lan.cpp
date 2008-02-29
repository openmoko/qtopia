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

static const char* wlanopts = "/etc/pcmcia/wlan-ng.opts";
static const char* wirelessopts = "/etc/pcmcia/wireless.opts";

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

#include "../addscrollbars.cpp"

Lan::Lan(Config& cfg, QWidget* parent) :
    LanBase(parent,0,TRUE),
    config(cfg)
{
    QPEApplication::setInputMethodHint(subnet,"netmask");
    QPEApplication::setInputMethodHint(ip,"netmask");
    QPEApplication::setInputMethodHint(broadcast,"netmask");
    QPEApplication::setInputMethodHint(gateway,"netmask");
    QPEApplication::setInputMethodHint(gateway,"netmask");
    QPEApplication::setInputMethodHint(dns1,"netmask");
    QPEApplication::setInputMethodHint(dns2,"netmask");
    
    ipGroup = new QButtonGroup( this );
    ipGroup->hide();
    ipGroup->insert( manualIP );
    ipGroup->insert( autoIP );
    connect( ipGroup, SIGNAL(clicked(int)), this, SLOT(ipSelect(int)) );
    ipGroup->setButton( 1 );

    if ( !QFile::exists(wlanopts) && !QFile::exists(wirelessopts) )
	tabs->setTabEnabled(wireless, FALSE);

    QBoxLayout *proxyLayout  = new QVBoxLayout( tab_2 ); 
    proxies = new ProxiesPage( tab_2 );
    proxyLayout->addWidget( proxies );

    addScrollBars(tabs);

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

    cfg.setGroup("Info");
    setCaption(cfg.readEntry("Name", caption()));
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
    tcpip->setEnabled(id == 0);
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
				     tr( "<qt>This name already exists. Please choose a different name.</qt>") );
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
	if ( wep_type->currentItem() == 0 ) {
	    wep = "NO";
	} else {
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
	}
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
    Q_CONST_UNUSED(config);
    Q_CONST_UNUSED(scheme);

    return FALSE;
}

static QString ngToWireless(Config& config, const QString& key)
{
    if ( key == "ESSID" ) {
	QString ssid = config.readEntry("SSID");
	if ( ssid.isEmpty() )
	    ssid = "any";
	return ssid;
    } else if ( key == "INFO" ) {
	config.setGroup("Info");
	QString nm = config.readEntry("Name");
	config.setGroup("Properties");
	return nm;
    } else if ( key == "MODE" ) {
	bool ah=config.readEntry("IS_ADHOC").lower()=="y";
	if ( ah )
	    return "Ad-Hoc";
	else
	    return "Managed";
    } else if ( key == "KEY" ) {
	QString wep = config.readEntry("WEP");
	if ( wep == "PP" ) {
	    return "s:"+config.readEntry("PRIV_GENSTR");
	} else {
	    QString v=config.readEntry("dot11WEPDefaultKey"+wep[1]);
	    v.replace(QRegExp(":"),"");
	    return v;
	}
    } else {
	return config.readEntry(key);
    }
}

bool Lan::writePcmciaNetworkOpts( Config &config, QString scheme )
{
    QString prev = "/etc/pcmcia/network.opts";

    QFile prevFile(prev);
    if ( !prevFile.open( IO_ReadOnly ) ) {
	QMessageBox::warning(0, tr("LAN Setup"), 
		     tr( "<qt>The file /etc/pcmcia/network.opts does not exist. Please restore this file.</qt>") );
	return FALSE;
    }

    QString tmp = prev + "-qpe-new";
    QFileInfo pcmciaInfo("/etc/pcmcia");
    if ( !pcmciaInfo.isWritable() )
    {
        QMessageBox::warning(0, tr("LAN Setup"),
	    tr( "<qt>You can not write to /etc/pcmcia/. Please check your permissions.</qt>") );
        return FALSE;
    }
    
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

    QString prevng = wlanopts;
    QString tmpng = prevng + "-qpe-new";
    QString prevwl = wirelessopts;
    QString tmpwl = prevwl + "-qpe-new";

    for (int ng=0; ng<2; ng++) {
	prevFile.setName(ng ? prevng : prevwl);
	if ( !prevFile.open( IO_ReadOnly ) ) {
	    if ( ng )
		prevng = QString::null;
	    else
		prevwl = QString::null;
	    break;
	}

	tmpFile.setName(ng ? tmpng : tmpwl);
	if ( !tmpFile.open( IO_WriteOnly ) ) {
            QMessageBox::warning(0, tr("LAN Setup"),
	        tr( "<qt>You can not write to /etc/pcmcia/. Please check your permissions.</qt>") );
	    return FALSE;
        }

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
			    QString v;
			    if ( ng )
				v = config.readEntry(k);
			    else
				v = ngToWireless(config,k);
			    line = "    " + k + "=" + Global::shellQuote(v);
			}
		    }
		} else {
		    if ( wline.left(scheme.length()+7) == scheme + ",*,*,*)" ) {
			found=TRUE;
		    } else if ( wline == "esac" || wline == "*,*,*,*)" ) {
			// end - add new entry

			wout << scheme << ",*,*,*)" << "\n";

			const char** f;
			if ( ng ) {
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
			    f = txtfields;
			} else {
			    static const char* txtfields[] = {
				"NWID", "CHANNEL", "FREQ", "SENS", "RATE", "RTS", "FRAG",
				"IWCONFIG", "IWSPY", "IWPRIV",

				// these need translation
				"ESSID", "INFO", "MODE", "KEY", 
				0
			    };
			    f = txtfields;
			}

			while (*f) {
			    QString v;
			    if ( ng )
				v = config.readEntry(*f,"");
			    else
				v = ngToWireless(config,*f);
			    wout << "    " << *f << "=" 
				    << Global::shellQuote(v) << "\n";
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
    }

    //system("cardctl suspend");
    system("cardctl eject");

    if ( system( "mv " + tmp + " " + prev ) )
	retval = FALSE;
    if ( !prevng.isNull() && system( "mv " + tmpng + " " + prevng ) )
	retval = FALSE;
    if ( !prevwl.isNull() && system( "mv " + tmpwl + " " + prevwl ) )
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
    else
        QDialog::reject();
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

#ifdef USE_SCHEMES
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
#endif


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
#ifdef USE_SCHEMES
	QStringList lans = findLans();
	QString scheme = findScheme(lans);
	if ( lans.count() > 1 ) {
	    services->insertStringList(lans);
	    for(QStringList::Iterator it=lans.begin(); it!=lans.end(); ++it)
		*it = toSchemeId(*it);
	    services->setCurrentItem(lans.findIndex(scheme));
	    SchemeChanger* sc = new SchemeChanger(this);
	    QObject::connect(services,SIGNAL(activated(const QString&)),
		sc,SLOT(changeLanSchemeSlot(const QString&)));
	} else {
	    services->hide();
	    services_label->hide();
	}
#else
        services->hide();
        services_label->hide();
#endif
	if ( !device[(int)device.length()-1].isDigit() )
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
#ifdef USE_SCHEMES
	QString n,s;
	{
	    cfg.setGroup("Info");
	    n = toSchemeId(cfg.readEntry("Name"));
            if ( n.isNull())
                return FALSE;
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
#else
        return true;
#endif
    }
    return FALSE;
}

bool LanImpl::isAvailable( Config& cfg ) const
{
    return NetworkInterface::isAvailable(cfg);
}


#include "lan.moc"
