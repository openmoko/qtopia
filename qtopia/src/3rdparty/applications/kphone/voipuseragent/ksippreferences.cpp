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
#include <qvbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qdir.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qmultilineedit.h>
#include <qsettings.h>

#include "../dissipate2/sipprotocol.h"
#include "../dissipate2/sipclient.h"
#include "kphone.h"
#include "callaudio.h"
#include "ksippreferences.h"

KSipPreferences::KSipPreferences( SipClient *client, CallAudio *ca, QWidget *parent, const char *name )
	: QTabDialog( parent, QString( name ) + tr("Global SIP Preferences") )
{
	c = client;
	audio = ca;

	QVBox *vboxadv = new QVBox();
	addTab( vboxadv, tr("Settings") );

	// Via hiding radio buttons
	hidebg = new QHButtonGroup( tr("Hide Via"), vboxadv );
	QRadioButton *nohide = new QRadioButton( tr("Don't Hide"), hidebg );
	hidebg->insert( nohide, NoHide );
	QRadioButton *hophide = new QRadioButton( tr("Request Next Hop", "Hop:a routing term as in hopping to the next routing server"), hidebg );
	hidebg->insert( hophide, HideHop );

	QRadioButton *routehide = new QRadioButton( tr("Request Full Route"), hidebg );
	hidebg->insert( routehide, HideRoute );

	// Set max forwards
	(void) new QLabel( tr("Max Forwards", "number of maximum hopping"), vboxadv );
	maxforwards = new QLineEdit( vboxadv );

	// PhoneBook
	(void) new QLabel( tr("XML-File for Phone Book:"), vboxadv );
	phonebook = new QLineEdit( vboxadv );

	// RegistrationExpiresTime
	(void) new QLabel( tr("Expire Time of Registration (second):", "timeout duration for an registration request to the server"), vboxadv );
	expires = new QLineEdit( vboxadv );

	// SubscribeExpiresTime
	(void) new QLabel( tr("Expire Time of Presence Subscription (second):", "Presence=availability for conversation"), vboxadv );
	subscribeExpires = new QLineEdit( vboxadv );

	// Socket
	QVBox *vboxs = new QVBox( );
	addTab( vboxs, "Socket" );

	// UDP / TCP
	socket = new QHButtonGroup( tr("Socket Protocol (Kphone restart needed to apply change)"), vboxs );

	QRadioButton *udp = new QRadioButton( tr("UDP"), socket );
	socket->insert( udp, UDP );

	QRadioButton *tcp = new QRadioButton( tr("TCP"), socket );
	socket->insert( tcp, TCP );

	// STUN
	stun = new QHButtonGroup( tr("Use STUN Server"), vboxs );
	QRadioButton *StunYes = new QRadioButton( tr("Yes"), stun );
	stun->insert( StunYes, stunYes );
	QRadioButton *StunNo = new QRadioButton( tr("No"), stun );
	stun->insert( StunNo, stunNo );

	// Symmetric signalling
	symmetric = new QHButtonGroup( tr("Symmetric Signalling"), vboxs );
	QRadioButton *SymmetricYes = new QRadioButton( tr("Yes"), symmetric );
	symmetric->insert( SymmetricYes, symmetricYes );
	QRadioButton *SymmetricNo = new QRadioButton( tr("No"), symmetric );
	symmetric->insert( SymmetricNo, symmetricNo );

	// Symmetric media
	symMedia = new QHButtonGroup( tr("Symmetric Media"), vboxs );
	QRadioButton *SymMediaYes = new QRadioButton( tr("Yes"), symMedia );
	symMedia->insert( SymMediaYes, symMediaYes );
	QRadioButton *SymMediaNo = new QRadioButton( tr("No"), symMedia );
	symMedia->insert( SymMediaNo, symMediaNo);

	(void) new QLabel( tr("STUN Server (address:port):"), vboxs );
	stunServerAddr = new QLineEdit( vboxs );

	// Request period
	(void) new QLabel( tr("Request Period for STUN Server (second):"), vboxs );
	stunRequestPeriod = new QLineEdit( vboxs );

	(void) new QLabel( tr("Media Min Port:"), vboxs );
	mediaMinPort = new QLineEdit( vboxs );

	(void) new QLabel( tr("Media Max Port:"), vboxs );
	mediaMaxPort = new QLineEdit( vboxs );

	// Call Preferences
	QVBox *vboxprefs = new QVBox( );
	addTab( vboxprefs, tr("Call Preferences") );

	// Busy message
	(void) new QLabel( tr("Busy Message:"), vboxprefs );
	busymsg = new QMultiLineEdit( vboxprefs );

	// Call Forwarding
	QVBox *vboxfor = new QVBox( );
	addTab( vboxfor, tr("Call Forwarding") );

	forbg = new QHButtonGroup( tr("Call Forwarding"), vboxfor );

	QRadioButton *nocforward = new QRadioButton( tr("Inactive"), forbg );
	forbg->insert( nocforward, FwInactive );

	QRadioButton *cforward = new QRadioButton( tr("Active"), forbg );
	forbg->insert( cforward, FwActive );

	// Forward to Uri
	(void) new QLabel( tr("Forward to:"), vboxfor );
	forwardaddr = new QLineEdit( vboxfor );

	// Forward to message
	(void) new QLabel( tr("Call Forward Message:"), vboxfor );
	forwardmsg = new QMultiLineEdit( vboxfor );

	setCancelButton();
	connect( this, SIGNAL( cancelButtonPressed() ), this, SLOT( slotCancel() ) );
	setOKButton();
	connect( this, SIGNAL( applyButtonPressed() ), this, SLOT( slotOk() ) );
	resetSettings();
}

KSipPreferences::~KSipPreferences( void )
{
}

void KSipPreferences::slotOk( void )
{
	slotApply();
	QDialog::accept();
}

void KSipPreferences::slotApply( void )
{
	switch( hidebg->id( hidebg->selected() ) ) {
		case HideHop:
			c->setHideViaMode( SipClient::HideHop );
			break;
		case HideRoute:
			c->setHideViaMode( SipClient::HideRoute );
			break;
		default:
		case NoHide:
			c->setHideViaMode( SipClient::DontHideVia );
			break;
	}
	if( forbg->id( forbg->selected() ) == FwActive ) {
		c->setCallForward( true );
	} else {
		c->setCallForward( false );
	}

	c->setCallForwardUri( SipUri( forwardaddr->text() ) );
	c->setCallForwardMessage( forwardmsg->text() );
	c->setMaxForwards( maxforwards->text().toUInt() );
	c->setBusyMessage( busymsg->text() );

	QSettings settings;
	settings.writeEntry( "/kphone/sip/callforward", c->getCallForward() );
	settings.writeEntry( "/kphone/sip/forwardaddr", c->getCallForwardUri().nameAddr() );
	settings.writeEntry( "/kphone/sip/forwardmsg", c->getCallForwardMessage() );
	settings.writeEntry( "/kphone/sip/maxforwards", c->getMaxForwards() );
	settings.writeEntry( "/kphone/sip/busymsg", c->getBusyMessage() );

	QString hvstr;
	switch( c->getHideViaMode() ) {
		default:
		case SipClient::DontHideVia: hvstr = "NoHide"; break;
		case SipClient::HideHop: hvstr = "HideHop"; break;
		case SipClient::HideRoute: hvstr = "HideRoute"; break;
	}
	settings.writeEntry( "/kphone/sip/hideviamode", hvstr );
	QString p = "/kphone/" + ((KPhone *)parentWidget())->getUserPrefix() + "/local/";
	settings.writeEntry( p + "/PhoneBook", phonebook->text() );
	if( expires->text() ) {
		settings.writeEntry( p + "/RegistrationExpiresTime", expires->text() );
	}
	if( subscribeExpires->text() ) {
		bool ok;
		int dec = subscribeExpires->text().toInt( &ok, 10 );
		if( ok ) {
			if( dec < 60 ) {
				subscribeExpires->setText( "60" );
			}
			settings.writeEntry( p + "/SubscribeExpiresTime", subscribeExpires->text() );
		}
	}
	switch( socket->id( socket->selected() ) ) {
		case TCP:
			settings.writeEntry( "/kphone/General/SocketMode", "TCP" );
			break;
		default:
			settings.writeEntry( "/kphone/General/SocketMode", "UDP" );
			break;
	}
	settings.writeEntry( "/kphone/STUN/StunServer", stunServerAddr->text() );
	settings.writeEntry( "/kphone/STUN/RequestPeriod", stunRequestPeriod->text() );
	switch( stun->id( stun->selected() ) ) {
		case stunYes:
			settings.writeEntry( "/kphone/STUN/UseStun", "Yes" );
			break;
		default:
			settings.writeEntry( "/kphone/STUN/UseStun", "No" );
			break;
	}

	settings.writeEntry("/kphone/Media/MinPort", mediaMinPort->text());
	settings.writeEntry("/kphone/Media/MaxPort", mediaMaxPort->text());

	switch( symmetric->id(symmetric->selected())) {
	case symmetricYes:
		settings.writeEntry("/kphone/Symmetric/Signalling", "Yes");
		c->setSymmetricMode(true);
		break;
	default:
		settings.writeEntry("/kphone/Symmetric/Signalling", "No");
		c->setSymmetricMode(false);
		break;
	}

	switch( symMedia->id(symMedia->selected())) {
	case symMediaYes:
		settings.writeEntry("/kphone/Symmetric/Media", "Yes");
		break;
	default:
		settings.writeEntry("/kphone/Symmetric/Media", "No");
		break;
	}
}

void KSipPreferences::slotCancel( void )
{
	resetSettings();
	QDialog::reject();
}

void KSipPreferences::resetSettings( void )
{
	forwardaddr->setText( c->getCallForwardUri().nameAddr() );
	forwardmsg->setText( c->getCallForwardMessage() );
	maxforwards->setText( QString::number( c->getMaxForwards() ) );
	busymsg->setText( c->getBusyMessage() );

	switch( c->getHideViaMode() ) {
		default:
		case SipClient::DontHideVia: hidebg->setButton( NoHide ); break;
		case SipClient::HideHop: hidebg->setButton( HideHop ); break;
		case SipClient::HideRoute: hidebg->setButton( HideRoute ); break;
	}

	if( c->getCallForward() ) {
		forbg->setButton( FwActive );
	} else {
		forbg->setButton( FwInactive );
	}
	QSettings settings;
	QString p = "/kphone/" + ((KPhone *)parentWidget())->getUserPrefix() + "/local/";
  	QString xmlFile = settings.readEntry( p + "/PhoneBook", "" );

	if( xmlFile.isEmpty() ) {
		if( ((KPhone *)parentWidget())->getUserPrefix().isEmpty() ) {
			xmlFile = QDir::homeDirPath() + "/.kphone-phonebook.xml";
		} else {
			xmlFile = QDir::homeDirPath() + "/.kphone" +
				((KPhone *)parentWidget())->getUserPrefix() + "phonebook.xml";
		}
	}
	phonebook->setText( xmlFile );
	QString strDefault;
	expires->setText( settings.readEntry( p + "/RegistrationExpiresTime",
		strDefault.setNum( constRegistrationExpiresTime ) ) );
	subscribeExpires->setText( settings.readEntry( p + "/SubscribeExpiresTime",
		strDefault.setNum( constSubscribeExpiresTime ) ) );
	if( settings.readEntry( "/kphone/STUN/UseStun", "No" ) == "Yes" ) {
		stun->setButton( stunYes );
	} else {
		stun->setButton( stunNo );
	}
	stunServerAddr->setText( settings.readEntry( "/kphone/STUN/StunServer", constStunServer ) );
	stunRequestPeriod->setText( settings.readEntry( "/kphone/STUN/RequestPeriod",
		strDefault.setNum( constStunRequestPeriod ) ) );
	if (settings.readEntry("/kphone/Symmetric/Signalling", "Yes") == "Yes") {
		symmetric->setButton(symmetricYes);
	} else {
		symmetric->setButton(symmetricNo);
	}

	if (settings.readEntry("/kphone/Symmetric/Media", "Yes") == "Yes") {
		symMedia->setButton(symMediaYes);
	} else {
		symMedia->setButton(symMediaNo);
	}
	mediaMinPort->setText( settings.readEntry("/kphone/Media/MinPort", strDefault.setNum(constMinPort)));
	mediaMaxPort->setText( settings.readEntry("/kphone/Media/MaxPort", strDefault.setNum(constMaxPort)));
	if( c->isTcpSocket() ) {
		socket->setButton( TCP );
	} else {
		socket->setButton( UDP );
	}
}
