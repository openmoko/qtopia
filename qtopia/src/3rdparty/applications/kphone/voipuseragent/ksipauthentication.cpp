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
#include <qmessagebox.h>
#include <qsettings.h>
#include <qlayout.h>

#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipuser.h"
#include "../dissipate2/sipregister.h"
#include "../dissipate2/sipclient.h"
#include "kphone.h"
#include "kphoneview.h"
#include "ksipauthentication.h"


KSipAuthenticationRequest::KSipAuthenticationRequest(
	const QString &server, const QString &sipuri, const QString &caption, QWidget *parent, const char * /*name*/ )
	: QDialog( parent, caption, true )
{
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );
	(void) new QLabel( tr("SipUri = ") + sipuri, vbox );
	(void) new QLabel( tr("Server = ") + server, vbox );
	(void) new QLabel( tr("Username:"), vbox );
	username = new QLineEdit( vbox );
	(void) new QLabel( tr("Password:"), vbox );
	password = new QLineEdit( vbox );
	password->setEchoMode( QLineEdit::Password );
	password->setFocus();
	savePassword = new QCheckBox( tr("Save password"), vbox );
	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	okPushButton = new QPushButton( this, tr("ok button") );
	okPushButton->setText( tr("OK") );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, tr("cancel button") );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ), this, SLOT( okClicked() ) );
	connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

KSipAuthenticationRequest::~KSipAuthenticationRequest( void )
{
}

void KSipAuthenticationRequest::setUsername( const QString &newUsername )
{
	username->setText( newUsername );
}

void KSipAuthenticationRequest::setPassword( const QString &newPassword )
{
	password->setText( newPassword );
}

QString KSipAuthenticationRequest::getUsername( void )
{
	return username->text();
}

QString KSipAuthenticationRequest::getPassword( void )
{
	return password->text();
}

void KSipAuthenticationRequest::okClicked( void )
{
	if (savePassword->isChecked())
		QSettings().writeEntry("/kphone/Registration/Password", getPassword());
	accept();
}

KSipAuthentication::KSipAuthentication()
{
	authreq = 0;
	execAuthreq = false;
}

KSipAuthentication::~KSipAuthentication( void )
{
}

void KSipAuthentication::authRequest( SipCallMember *member )
{
	if( member->getAuthState() != SipCallMember::authState_AuthenticationRequired &&
	    member->getAuthState() != SipCallMember::authState_AuthenticationRequiredWithNewPassword ) {
		return;
	}
	QString u = member->getCall()->getProxyUsername();
	QString p = member->getCall()->getPassword();
	if( execAuthreq ) return;
	if( p.isEmpty() ||
	    member->getAuthState() == SipCallMember::authState_AuthenticationRequiredWithNewPassword ) {
		QString proxy = member->getCall()->getSipProxy();
		SipUri localuri = member->getCall()->localAddress();
		//if( !authreq ) {
		//	authreq = new KSipAuthenticationRequest( proxy, localuri.uri(), QString::null );
		//}
		//authreq->setUsername( u );
		//authreq->setPassword( p );
		//execAuthreq = true;
		//if( authreq->exec() ) {
		//	u = authreq->getUsername();
		//	p = authreq->getPassword();
		//	if( u.isEmpty() || p.isEmpty() ) {
		//		return;
		//	}
		//	member->getCall()->setPassword( p );
		//	execAuthreq = false;
		//} else {
		//	execAuthreq = false;
		//	return;
		//}
	}

	switch( member->getCallMemberType() ){
		case SipCallMember::Subscribe:
			member->sendRequestSubscribe( u, p );
			break;
		case SipCallMember::Notify:
			member->sendRequestNotify( u, p );
			break;
		case SipCallMember::Message:
			member->sendRequestMessage( u, p );
			break;
		case SipCallMember::Invite:
			member->sendRequestInvite( u, p );
			break;
		default:
			break;
	}
}
