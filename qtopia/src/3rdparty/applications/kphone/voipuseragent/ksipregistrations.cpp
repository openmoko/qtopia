/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/**********************************************************************
** This file is part of the KPhone project.
** 
** KPhone is a SIP (Session Initiation Protocol) user agent for Linux, with
** which you can initiate VoIP (Voice over IP) connections over the Internet,
** send Instant Messages and subscribe your friends' presence information.
** Read INSTALL for installation instructions, and CHANGES for the latest
** additions in functionality and COPYING for the General Public License
** (GPL).
** 
** More information about Wirlab available at http://www.wirlab.net/
** 
** Note: "This program is released under the GPL with the additional
** exemption that compiling, linking, and/or using OpenSSL is allowed."
***********************************************************************/
#include <stdio.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
//#include <qmessagebox.h>
#include <qsettings.h>
#include <qlayout.h>

#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipuser.h"
#include "../dissipate2/sipregister.h"
#include "../dissipate2/sipclient.h"
#include "kphone.h"
#include "kphoneview.h"
#include "ksipauthentication.h"
#include "ksipregistrations.h"
#include <voipuseragent.h>
extern VoIPUserAgent *pVoipUserAgent;

KSipIdentityEdit::KSipIdentityEdit( QWidget *parent, const char *name, QObject *reg )
	: QDialog( parent, QString( name ) + tr("Identity Editor"), true )
{
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );

	(void) new QLabel( tr("Full Name:"), vbox );
	fullname = new QLineEdit( vbox );
	fullname->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	(void) new QLabel( tr("User Part of SIP URL:"), vbox );
	username = new QLineEdit( vbox );
	(void) new QLabel( tr("Host Part of SIP URL:"), vbox );
	hostname = new QLineEdit( vbox );
	(void) new QLabel( tr("Outbound Proxy (optional):"), vbox );
	sipProxy = new QLineEdit( vbox );
	(void) new QLabel( tr("Authentication Username (optional):"), vbox );
	sipProxyUsername = new QLineEdit( vbox );
	(void) new QLabel( tr("q-value between 0.0-1.0 (optional):"), vbox );
	qValue = new QLineEdit( vbox );
	autoRegister = new QCheckBox( tr("Auto Register"), vbox );
	labelRegister = new QLabel( "", vbox );
	buttonRegister = new QPushButton( tr("Register"), vbox );
	connect( buttonRegister, SIGNAL( clicked() ), reg, SLOT( changeRegistration() ) );

	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	helpPushButton = new QPushButton( this, tr("help button") );
	helpPushButton->setText( tr("&help...") );
	buttonBox->addWidget( helpPushButton );
	QSpacerItem *spacer = new QSpacerItem(
		0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonBox->addItem( spacer );
	okPushButton = new QPushButton( this, tr("ok button") );
	okPushButton->setText( tr("OK") );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, tr("cancel button") );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ),
		this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ),
		this, SLOT( slotCancel() ) );
	connect( helpPushButton, SIGNAL( clicked() ),
		parent, SLOT( showHelp() ) );
}

KSipIdentityEdit::~KSipIdentityEdit( void )
{
}

void KSipIdentityEdit::updateState( KsipState state )
{
	if( state == OFFLINE ) {
		buttonRegister->setText( tr("Register : not registered") );
		buttonRegister->setEnabled( false );
	} else if( state == REG ) {
		labelRegister->setText( tr("Registration : registered"));
		buttonRegister->setText( tr("Unregister") );
		buttonRegister->setEnabled( true );
	} else if ( state == UNREG ) {
		labelRegister->setText( tr("Registration : not registered"));
		buttonRegister->setText( tr("Register") );
		buttonRegister->setEnabled( true );
	} else {
          labelRegister->setText( tr("Registration : ") );
        }
}

QString KSipIdentityEdit::getFullname( void ) const
{
	return fullname->text();
}

QString KSipIdentityEdit::getUsername( void ) const
{
	return username->text();
}

QString KSipIdentityEdit::getHostname( void ) const
{
	return hostname->text();
}

QString KSipIdentityEdit::getSipProxy( void ) const
{
	return sipProxy->text();
}

QString KSipIdentityEdit::getSipProxyUsername( void ) const
{
	return sipProxyUsername->text();
}

QString KSipIdentityEdit::getQvalue( void ) const
{
	bool ok;
	float f = qValue->text().toFloat( &ok );
	if( ok && f >= 0 && f <= 1 ) {
		return qValue->text();
	}
	return "";
}

QString KSipIdentityEdit::getUri( void ) const
{
	return "\"" + fullname->text() + "\" <sip:" + username->text() + "@" + hostname->text() + ">";
}

void KSipIdentityEdit::setFullname( const QString &newFullname )
{
	fullname->setText( newFullname );
}

void KSipIdentityEdit::setUsername( const QString &newUsername )
{
	username->setText( newUsername );
}

void KSipIdentityEdit::setHostname( const QString &newHostname )
{
	hostname->setText( newHostname );
}

void KSipIdentityEdit::setSipProxy( const QString &newSipProxy )
{
	sipProxy->setText( newSipProxy );
}

void KSipIdentityEdit::setQvalue( const QString &qvalue )
{
	qValue->setText( qvalue );
}

void KSipIdentityEdit::setSipProxyUsername( const QString &newSipProxyUsername )
{
	sipProxyUsername->setText( newSipProxyUsername );
}

void KSipIdentityEdit::slotOk( void )
{
	if( username->text() == "" || hostname->text() == "" ) {
		QDialog::reject();
	} else {
		update();
		QDialog::accept();
	}
}

void KSipIdentityEdit::slotCancel( void )
{
	QDialog::reject();
}

KSipRegistrations::KSipRegistrations(
		SipClient *client, KPhoneView *phoneView, QWidget *p, const char *prefix )
{
	parent = p;
	c = client;
	v = phoneView;
	userPrefix = prefix;
	useStun = false;

  #ifdef KPHONE_APP
	edit = new KSipIdentityEdit( parent, userPrefix.latin1(), this );
  connect( edit, SIGNAL( update() ), this, SLOT( update() ) );
  #endif

  sipreg = 0;
	QString uristr;
	QString str;
	SipUri uri;
	QSettings settings;
	QString pp = "/kphone/" + userPrefix + "/local/";
	expires = settings.readNumEntry( pp + "/RegistrationExpiresTime", constRegistrationExpiresTime );

	if( expires == 0 ) {
		expires = -1;
	}
	pp = "/kphone/" + userPrefix + "Registration/";
	if( settings.readEntry( pp + "/SipUri", "" ) != "" ) {
		uristr = settings.readEntry( pp + "/SipUri" );
		uri = SipUri( uristr );
		u = c->getUser( uri );
		if( u == NULL ) {
			u = new SipUser( c, uri );
		}
		stunSrv = "";
		if( settings.readEntry( "/kphone/STUN/UseStun", "" ) == "Yes" ) {
			useStun = true;
			stunSrv = settings.readEntry( "/kphone/STUN/StunServer", constStunServer );
			if( stunSrv.isEmpty() ) {
				QString dname = u->getMyUri()->getHostname();
				stunSrv = dname;
				QString srv = client->getSRV( QString( "_stun._udp." ) + dname );
				if( !srv.isEmpty() ) {
					stunSrv = srv;
				}
				stunSrv += ":3478";
			} else {
				if( !stunSrv.contains( ':' ) ) {
					stunSrv += ":3478";
				}
			}
		}
		uristr = "";
		if( settings.readEntry( pp + "/SipServer", "" ) != "" ) {
			uristr = settings.readEntry( pp + "/SipServer" );
		}
		QString qvalue = settings.readEntry( pp + "/qValue", "" );
		sipreg = new SipRegister( u, SipUri( uristr ), expires, qvalue );
		connect( sipreg, SIGNAL( statusUpdated() ),
			this, SLOT( registerStatusUpdated() ) );
		v->updateIdentity( u, sipreg );
		c->updateIdentity( u, sipreg->getOutboundProxy() );
		str = settings.readEntry( pp + "/UserName" );
		u->getMyUri()->setProxyUsername( str );
		str = settings.readEntry( pp + "/Password" );
		u->getMyUri()->setPassword( str );
		str = settings.readEntry( pp + "/AutoRegister" );
		if( str == "Yes" ) {
			autoRegister = true;
			if( useStun ) {
				sipreg->setAutoRegister( true );
			} else {
				sipreg->requestRegister();
			}
		} else {
			autoRegister = false;
			sipreg->setAutoRegister( false );
		}
	} else {
		editRegistration();
	}
}

KSipRegistrations::~KSipRegistrations( void )
{
}

void KSipRegistrations::showIdentity( void )
{
	editRegistration();
}

void KSipRegistrations::save( void )
{
}

void KSipRegistrations::editRegistration( void )
{

  #ifdef KPHONE_APP
  if( sipreg ) {
		setRegisterState();
		edit->setFullname( u->getUri().getFullname() );
		edit->setUsername( u->getUri().getUsername() );
		edit->setHostname( u->getUri().getHostname() );
		edit->setSipProxy( sipreg->getOutboundProxy() );
		edit->setSipProxyUsername( u->getUri().getProxyUsername() );
		edit->setAutoRegister( autoRegister );
		edit->setQvalue( sipreg->getQvalue() );
	} else {
		edit->updateState( OFFLINE );
		edit->setFullname( "" );
		edit->setUsername( "" );
		edit->setHostname( "" );
		edit->setSipProxy( "" );
		edit->setSipProxyUsername( "" );
		edit->setQvalue( "" );
		edit->setAutoRegister( true );
	}
	edit->show();
  #endif

}

void KSipRegistrations::update( void )
{
    QSettings settings;
    QString p = "/kphone/" + userPrefix + "Registration/";

    autoRegister = settings.readEntry( p + "/AutoRegister") == "Yes";
    QString s = settings.readEntry( p + "/SipServer", "" );

    QString uristr = settings.readEntry( p + "/SipUri", "" );
    SipUri uri = SipUri( uristr );
    u = c->getUser( uri );
    if( u == NULL ) {
            u = new SipUser( c, uri );
    }
    uristr = settings.readEntry( p + "/SipServer", "" );
    QString qvalue = settings.readEntry( p + "qValue", "" );
    sipreg = new SipRegister( u, SipUri( uristr ), expires, qvalue );
    connect( sipreg, SIGNAL( statusUpdated() ),
            this, SLOT( registerStatusUpdated() ) );
    v->updateIdentity( u, sipreg );
    c->updateIdentity( u, sipreg->getOutboundProxy() );

    QString str = settings.readEntry( p + "/UserName", "" );
    u->getMyUri()->setProxyUsername( str );

    str = settings.readEntry( p + "/Password", "" );
    u->getMyUri()->setPassword( str );

    if( autoRegister ) {
            sipreg->requestRegister();
    } else {
            sipreg->setAutoRegister( false );
    }
    if( autoRegister ) {
            changeRegistration();
    }
}

void KSipRegistrations::changeRegistration( void )
{
	if( sipreg ) {
		if( sipreg->getRegisterState() == SipRegister::Connected ) {
			sipreg->requestClearRegistration();
		} else {
			sipreg->updateRegister();
			v->setContactsOnline();
		}
	}
}

void KSipRegistrations::unregAllRegistration( void )
{
	if( sipreg ) {
		if( sipreg->getRegisterState() == SipRegister::Connected ) {
			sipreg->requestClearRegistration();
		}
	}
}

 // PD
extern void RegistrationStatusCallback(int);

void KSipRegistrations::setRegisterState( void )
{
  #ifdef KPHONE_APP
	switch( sipreg->getRegisterState() ) {
		case SipRegister::NotConnected:
			edit->updateState(  UNREG );
			break;
		case SipRegister::TryingServer:
		case SipRegister::TryingServerWithPassword:
			edit->updateState( PROC_TRY );
			break;
		case SipRegister::AuthenticationRequired:
		case SipRegister::AuthenticationRequiredWithNewPassword:
			edit->updateState( AUTHREQ );
			break;
		case SipRegister::Connected:
			edit->updateState( REG );
			break;
		case SipRegister::Disconnecting:
			edit->updateState( PROC_UNREG );
			break;
		case SipRegister::Reconnecting:
			edit->updateState( PROC_REG );
			break;
 }
  #else
  // VoIP Callback
  if(pVoipUserAgent)
	pVoipUserAgent->RegistrationStatusCallback(sipreg->getRegisterState());
  #endif
}

void KSipRegistrations::registerStatusUpdated( void )
{
	setRegisterState();
	if( sipreg->getRegisterState() != SipRegister::AuthenticationRequired &&
			sipreg->getRegisterState() != SipRegister::AuthenticationRequiredWithNewPassword ) {
		return;
	}
	QString authtype;
	switch( sipreg->getAuthenticationType() ) {
		case SipRegister::DigestAuthenticationRequired:
			authtype = tr("Digest Authentication Request"); break;
		case SipRegister::BasicAuthenticationRequired:
			authtype = tr("Basic Authentication Request"); break;
		case SipRegister::ProxyDigestAuthenticationRequired:
			authtype = tr("Proxy Digest Authentication Request"); break;
		case SipRegister::ProxyBasicAuthenticationRequired:
			authtype = tr("Proxy Basic Authentication Request"); break;
	}
	QString server = sipreg->getServerUri().proxyUri();
	QString sipuri = u->getUri().uri();
	//KSipAuthenticationRequest authreq( server, sipuri, userPrefix + authtype );
	QString username = sipreg->getRegisterCall()->getProxyUsername();
	//authreq.setUsername( username );
	QString password = sipreg->getRegisterCall()->getPassword();
	if( password.isEmpty() || sipreg->getRegisterState() == SipRegister::AuthenticationRequiredWithNewPassword ) {
		//if( authreq.exec() ) {
		//	if( authreq.getUsername().isEmpty() || authreq.getUsername().isEmpty() ) {
		//		return;
		//	}
		//	sipreg->getRegisterCall()->setProxyUsername( authreq.getUsername() );
		//	sipreg->getRegisterCall()->setPassword( authreq.getPassword() );
		//	sipreg->requestRegister( authreq.getUsername(), authreq.getPassword() );
		//}
	} else {
		sipreg->requestRegister( username, password );
	}
}


int KSipRegistrations::RegisterVoIPSip(const QString &/*username*/, const QString &/*password*/)
{
  if(sipreg)
    sipreg->requestRegister();
  
  return 0;
}

//int KSipRegistrations::UnRegisterVoIPSip()
//(const QString &username, const QString &password)
//{
//  if(sipreg)
//    sipreg->requestRegister();
//}
