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
#include <stdlib.h>
#include <qtimer.h>
#include <qsettings.h>
#include <qmenubar.h>
//#include <qmessagebox.h>
#include <qapplication.h>

#include "../dissipate2/sipuser.h"
#include "../dissipate2/sipclient.h"
#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipregister.h"
#include "callaudio.h"
#include "audiocontrol.h"
#include "ksipregistrations.h"
#include "kphoneview.h"
#include "ksippreferences.h"
#include "ksipoptions.h"
#include "kphonebook.h"
#include "kphone.h"

KPhone::KPhone( unsigned int listenport, QString prefix, bool looseRoute, bool strictRoute ) : QMainWindow(0)
{
	QString uristr;
	if( Sip::getLocalAddress().isEmpty() ) {
		exit(0);
	}
	userPrefix = "";
	if( !prefix.isEmpty() ) {
		setCaption( "kphone - " + prefix );
		userPrefix = "_" + prefix + "_";
	}
	QSettings settings;
	Sip::setLocalAddress( settings.readEntry(
		"/kphone/dissipate_addr", Sip::getLocalAddress() ) );
	QString socketStr = settings.readEntry( "/kphone/General/SocketMode", "UDP" );
	
	client = new SipClient( 0, 0, listenport, looseRoute, strictRoute, socketStr );
	QString p = "/kphone/" + getUserPrefix() + "Registration/";
	uristr = settings.readEntry( p + "SipUri" );

	if( settings.readEntry( p + "/UseProxyDial", "Yes" ) == "Yes" ) {
		client->setUseProxyDial( true );
	} else {
		client->setUseProxyDial( false );
	}
	client->setCallForward( settings.readBoolEntry( p + "/callforward", false ) );
	client->setCallForwardUri( SipUri( settings.readEntry( p + "/forwardaddr", QString::null ) ) );
	client->setCallForwardMessage( settings.readEntry( p + "/forwardmsg", QString::null ) );
	client->setMaxForwards( settings.readNumEntry( p + "/maxforwards", 0 ) );
	client->setBusyMessage( settings.readEntry( p + "/busymsg", QString::null ) );
	if( settings.readEntry( "/kphone/Symmetric/Signalling", "Yes" ) == "Yes" ) {
		client->setSymmetricMode( true );
	}
	QString hvstr = settings.readEntry( "/kphone/Symmetric/hideviamode", "NoHide" );
	if( hvstr == "NoHide" ) { client->setHideViaMode( SipClient::DontHideVia ); }
	else if( hvstr == "HideHop" ) { client->setHideViaMode( SipClient::HideHop ); }
	else if( hvstr == "HideRoute" ) { client->setHideViaMode( SipClient::HideRoute ); }
	user = new SipUser( client, SipUri( uristr ) );

	// Initially, none of the dialogs exist yet
	qdial = 0;
	userinfo = 0;
	audiocontrol = 0;
	videocontrol = 0;
	registrations = 0;
	sipprefs = 0;
	queryoptions = 0;
	testoptions = 0;
	view = new KPhoneView( client, user, this, getUserPrefix() );
	audio = new CallAudio( view );
	if( uristr != QString::null ) {
		p = "/kphone/" + getUserPrefix() + "Registration/";
		QString userdefaultproxy = settings.readEntry( p + "SipServer", QString::null );
		if( userdefaultproxy.lower() == "sip:" ) {
			userdefaultproxy = "";
		}
		view->updateIdentity( user );
		client->updateIdentity( user, userdefaultproxy );
	}

	registrations = new KSipRegistrations( client, view, this, getUserPrefix() );
	view->identities( registrations );

	setCentralWidget( view );

	QPopupMenu *popup = new QPopupMenu;
	popup->insertItem( tr("New Call..."), view, SLOT( makeNewCall() ), CTRL + Key_N );
	popup->insertItem( tr("New Video Call..."), view, SLOT( makeVideoCall() ), CTRL + Key_V );
	popup->insertItem( tr("Identity..."), this, SLOT( showRegistrations() ), CTRL + Key_I );

//#test
//	popup->insertItem( "Options...", this, SLOT( showQueryOptions() ) );
//	popup->insertItem( "Test...", this, SLOT( showTestOptions() ) );

	popup->insertItem( tr("&Quit"), this, SLOT( kphoneQuit() ), CTRL + Key_Q );
	menuBar()->insertItem( tr("&File"), popup );

	preferencesMenu = new QPopupMenu;
	preferencesMenu->insertItem( tr("SIP Preferences..."), this, SLOT( showGlobalPreferences() ) );
	preferencesMenu->insertItem( tr("Audio Preferences..."), this, SLOT( showAudioPreferences() ) );
	preferencesMenu->insertItem( tr("Video Preferences..."), this, SLOT( showVideoPreferences() ) );
	preferencesMenu->insertItem( tr("Phone Book"), view, SLOT( showPhoneBook() ) );
	menuBar()->insertItem( tr("&Preferences"), preferencesMenu );
	menuBar()->insertSeparator();
	popup = new QPopupMenu;
	popup->insertItem( tr("Help"), this, SLOT( showHelp() ) );
	popup->insertItem( tr("About KPhone"), this, SLOT( showAboutKPhone() ) );
	menuBar()->insertItem( tr("&Help"), popup );

	if( registrations->getUseStun() ) {
		view->setStunSrv( registrations->getStunSrv() );
		client->sendStunRequest( registrations->getStunSrv() );
		QTimer *stun_timer = new QTimer( this );
		connect( stun_timer, SIGNAL( timeout() ), this, SLOT( stun_timerTick() ) );
		p = "/kphone/" + getUserPrefix() + "Registration/";
		int timeout = settings.readNumEntry( p + "RequestPeriod", constStunRequestPeriod );
		if( timeout > 0 ) {
			stun_timer->start( timeout * 1000 );
		}
	}
	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), this, SLOT( timerTick() ) );
	timer->start( 1 );
	resize( 160, 250 );
}

KPhone::~KPhone( void )
{
}

void KPhone::timerTick( void )
{
	client->doSelect( false );
}

void KPhone::stun_timerTick( void )
{
	client->sendStunRequest();
}

void KPhone::showUsers( void )
{
}

QString KPhone::getUserPrefix( void )
{
	return userPrefix;
}

void KPhone::showUserInfo( void )
{
}

void KPhone::showAudioPreferences( void )
{
	if( !audiocontrol ) {
		audiocontrol = new AudioControl( this, getUserPrefix() );
	}
	audiocontrol->show();
}

void KPhone::showVideoPreferences( void )
{
	if( !videocontrol ) {
		videocontrol = new VideoControl( this, getUserPrefix() );
	}
	videocontrol->show();
}

void KPhone::showRegistrations( void )
{
	registrations->showIdentity();
}

void KPhone::showGlobalPreferences( void )
{
	if( !sipprefs ) {
		sipprefs = new KSipPreferences( client, audio, this, getUserPrefix() );
	}
	sipprefs->show();
}

void KPhone::showQueryOptions( void )
{
	if( !queryoptions ) {
		queryoptions = new KSipOptions( user );
	}
	queryoptions->show();
}

void KPhone::showTestOptions( void )
{
	if( !testoptions ) {
		testoptions = new KTestOptions( client );
	}
	client->setTest( true );
	testoptions->show();
}

void KPhone::showHelp( void )
{
//       PD
//       QMessageBox mb( "KPhone",tr(
//		"When starting KPhone for the first time, you should enter your identity information in the window appearing.\n"
//		"The information required for successful sipping are 'Full Name', 'User part of SIP URL' and 'Host part of SIP URL'.\n"
//		"Note for KPhone 3.xx users: copy '~/.kde/share/config/kphonerc' file to '~/.qt' directory.\n"
//		"\n"
//		"If you have a NAPTR/SRV enabled DNS-server you can leave the 'Outbound Proxy' and 'Authentication Username' fields empty,\n"
//		"as KPhone will automatically search for the server information by the 'Host part of SIP URL' and prompt the username/password.\n"
//		"\n"
//		"Mark the auto register selection if you wish KPhone to automatically register your identity on application start.\n"
//		"If you want to use multiple identities, start KPhone with the '-u' option and the username of your selection.\n"
//		"For example:  'kphone -u jsmith'.\n"
//		"\n"
//		"If you are behind a NAT (Network Address Translation) device, you should go to Preferences -> SIP Preferences -> Socket and\n"
//		"check the 'Use STUN Server'\n"
//		"\n"
//		"The right button of your mouse is your best friend in KPhone's main UI. By clicking the right mouse button (RMB) in the main\n"
//		"window you get a menu including all important functions of KPhone. You should edit the Phone Book and enter all your favourite\n"
//		"contacts there. Of course this is not mandatory but it increases your efficiency with KPhone.\n"
//		"\n"
//		"When adding contacts to your Phone Book, simply click Add and enter the person's real name, SIP identity and the optional\n"
//		"description. If you want your contacts to be displayed in the main UI, just check the 'Add Identity ...' box. After editing your\n"
//		"contacts you can return to the main UI by clicking Hide. All your selected contacts should now appear there. Making calls and \n"
//		"sending instant messages to your contacts is easy with the RMB. Just click and select the function which you'd like to initiate.\n"),
//		QMessageBox::NoIcon,
//		QMessageBox::Ok  | QMessageBox::Default,
//		QMessageBox::NoButton,
//		QMessageBox::NoButton );
//	mb.exec();


}

void KPhone::showAboutKPhone( void )
{
//	QMessageBox::about( this, tr("About KPhone"),
//		"KPhone " + KPhoneVersion + " " + tr("is a 'Voice Over Internet' phone") + "\n\n" +
//		tr("Copyright (c) 2000 Billy Biggs") + "<bbiggs@div8.net>\n" +
//		tr("Copyright (c) 2004 Wirlab") + "<kphone@wirlab.net>\n\n" +
//		tr("KPhone is released under the GPL") + "\n\n" +
//		tr("For more information, see") + "\n" +
//		"http://www.wirlab.net/kphone" + "\n" );
}

void KPhone::toggleVisibility()
{
	if ( this->isVisible() ) {
		this->hide();
	} else {
		this->show();
	}
}

void KPhone::trayMenuRequested( const QPoint& pos )
{
	QPopupMenu* trayMenu = new QPopupMenu(this);
	trayMenu->insertItem( isVisible() ? "&Hide" : "S&how", this, SLOT( toggleVisibility() ) );
	trayMenu->insertItem( view->getState() ? "&Offline" : "&Online" , view, SLOT( buttonOffOnlineClicked() ) );
	trayMenu->insertSeparator();
	trayMenu->insertItem( "&Quit", this, SLOT( kphoneQuit() ) );
	trayMenu->exec(pos);
	delete trayMenu;
}

void KPhone::kphoneQuit( void )
{
	view->kphoneQuit();
	quitTimer = new QTimer( this );
	connect( quitTimer, SIGNAL( timeout() ), qApp, SLOT( quit() ) );
	quitTimer->start( quitTime );
	hide();
}

void KPhone::closeEvent( QCloseEvent* ce )
{
	ce->ignore();
	kphoneQuit();
}

KPhoneView *KPhone::getView( void )
{
	return view;
}
