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
#include <qlayout.h>
#include <qsplitter.h>
#include <qlabel.h>
#include <qcheckbox.h>
//PD #include <qmessagebox.h>
#include <qsettings.h>
#include <qvbox.h>
#include <qdir.h>

#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipclient.h"
#include "../dissipate2/sipstatus.h"
#include "../dissipate2/siptransaction.h"
#include "../dissipate2/sipmessage.h"
#include "calllistview.h"
#include "kcallwidget.h"
#include "callaudio.h"
#include "kphone.h"
#include "ksipauthentication.h"
#include "kphonebook.h"
#include "kphoneview.h"
#include <voipuseragent.h>

extern VoIPUserAgent *pVoipUserAgent;

ContactParser::ContactParser( void )
	: QXmlDefaultHandler()
{
}

bool ContactParser::startDocument()
{
	return TRUE;
}

bool ContactParser::startElement( const QString&, const QString&,
		const QString& qName,
		const QXmlAttributes& attributes)
{
	if( qName == "identity" ) {
		QString description = attributes.value( "description" );
		QString uri = attributes.value( "uri" );
		QString contact = attributes.value( "contact" );
		if( uri.left( 4 ).lower() != "tel:" && uri.left( 4 ).lower() != "sip:" ) {
			uri = "sip:" + uri;
		}
		if( contact.lower() != "no" ) {
			if( !description.isEmpty() ) {
				uri += ";" + description;
			}
			contacts.append( uri );
		}
	}
	return TRUE;
}

bool ContactParser::endElement( const QString&, const QString&, const QString& )
{
	return TRUE;
}

KPhoneView::KPhoneView( SipClient *c, SipUser *u, QWidget *parent, const char *name )
	: QWidget( parent, name ), client( c ), user( u )
{
	reg = 0;
	kphone = (KPhone*)parent;
	buttonSetOffline = false;
	setSubscribeOffline = false;
	connect( client, SIGNAL( updateSubscribes() ),
		this, SLOT( updateSubscribes() ) );
	connect( client, SIGNAL( incomingInstantMessage( SipMessage * ) ),
		this, SLOT( incomingInstantMessage( SipMessage * ) ) );
	connect( client, SIGNAL( incomingSubscribe( SipCallMember *, bool ) ),
		this, SLOT( incomingSubscribe( SipCallMember *, bool ) ) );
	connect( client, SIGNAL( incomingCall( SipCall *, QString ) ),
		this, SLOT( incomingCall( SipCall *, QString ) ) );
	connect( client, SIGNAL( hideCallWidget( SipCall * ) ),
		this, SLOT( hideCallWidget( SipCall * ) ) );
	QVBoxLayout *vbox = new QVBoxLayout( this );
	QGridLayout *headergrid = new QGridLayout( 4, 5 );
	vbox->insertLayout( 0, headergrid );
	buttonSipUri = new QPushButton( tr("Set Identity"), this );
	buttonSipUri->setMinimumWidth( kphoneMinimunWidht );
	headergrid->addMultiCellWidget( buttonSipUri, 0, 0, 0, 4 );
	connect( buttonSipUri, SIGNAL( clicked() ),
		this, SLOT( showIdentities() ) );
	lineeditCall = new QLineEdit( this );
	lineeditCall->setFocus();
	connect( lineeditCall, SIGNAL( returnPressed() ),
		this, SLOT( makeNewCall() ) );
	headergrid->addMultiCellWidget( lineeditCall, 1, 1, 0, 2 );
	buttonNewCall = new QPushButton( "", this );
	buttonNewCall->setFixedWidth( 30 );
	QIconSet icon;
	icon.setPixmap(SHARE_DIR "/icons/mini-kphone.png", QIconSet::Automatic );
	buttonNewCall->setIconSet( icon );
	headergrid->addWidget( buttonNewCall, 1, 3 );
	connect( buttonNewCall, SIGNAL( clicked() ),
		this, SLOT( makeNewCall() ) );
	buttonVideoCall = new QPushButton( "", this );
	buttonVideoCall->setFixedWidth( 30 );
	QIconSet icon3;
	icon3.setPixmap(SHARE_DIR "/icons/videocall.png", QIconSet::Automatic );
	buttonVideoCall->setIconSet( icon3 );
	headergrid->addWidget( buttonVideoCall, 1, 4 );
	connect( buttonVideoCall, SIGNAL( clicked() ),
		this, SLOT( makeVideoCall() ) );
	callcount = 1;
	clist = new ContactsListView( client, this );
	headergrid->addMultiCellWidget( clist, 3, 3, 0, 4 );
	clist->addMenuItem( tr("Send Message"), this, SLOT( contactSendMessage() ) );
	clist->addMenuItem( tr("Call"), this, SLOT( contactCall() ) );
	clist->addMenuItem( tr("Video Call"), this, SLOT( contactVideoCall() ) );
	clist->addMenuItem( tr("Phone Book"), this, SLOT( showPhoneBook() ) );
	connect( clist, SIGNAL( doubleClicked( QListViewItem * ) ),
		this, SLOT( contactDoubleClicked( QListViewItem * ) ) );
	connect( client, SIGNAL( callListUpdated() ),
		this, SLOT( presenceStatusListUpdation() ) );
	buttonOffOnline = new QPushButton( "", this );
	buttonOffOnline->setFixedWidth( 30 );
	QIconSet icon1;
	icon1.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
	buttonOffOnline->setIconSet( icon1 );
	connect( buttonOffOnline, SIGNAL( clicked() ),
		this, SLOT( buttonOffOnlineClicked() ) );
	headergrid->addWidget( buttonOffOnline, 4, 0 );
        stateComboBox = new QComboBox( TRUE, this, "" );
	QString online = tr("Online");
	QString busy = tr("Busy");
	QString berightback = tr("Be Right Back");
	QString away = tr("Away");
	QString onthephone = tr("On The Phone");
	QString outtolunch = tr("Out To Lunch");
	static const char* items[] = {
		online, busy,berightback, away,
		onthephone, outtolunch, 0 };
        stateComboBox->insertStrList( items );
        headergrid->addMultiCellWidget( stateComboBox, 4, 4, 1, 3 );
	buttonUpdate = new QPushButton( "", this );
	buttonUpdate->setFixedWidth( 30 );
	QIconSet icon2;
	icon2.setPixmap(SHARE_DIR "/icons/enter.png", QIconSet::Automatic );
	buttonUpdate->setIconSet( icon2 );
	connect( buttonUpdate, SIGNAL( clicked() ),
		this, SLOT( buttonUpdateClicked() ) );
	headergrid->addWidget( buttonUpdate, 4, 4 );
	QString label;
	QString uristr;
	sipauthentication = new KSipAuthentication();
	QSettings settings;

	SipCall *newcall;
	SipCallMember *member;
	QString p = "/kphone/" + getUserPrefix() + "/local/";
	QString file = settings.readEntry( p + "/PhoneBook", "" );
	if( file.isEmpty() ) {
		if( getUserPrefix().isEmpty() ) {
			file = QDir::homeDirPath() + "/.kphone-phonebook.xml";
		} else {
			file = QDir::homeDirPath() + "/.kphone" +
				getUserPrefix() + "phonebook.xml";
		}
	}
	QFile xmlFile( file );
	QXmlInputSource source( xmlFile );
	QXmlSimpleReader reader;
	ContactParser parser;
	reader.setContentHandler( &parser );
	reader.parse( source );
	QStringList sl = parser.getListContacts();
	for ( QStringList::Iterator it = sl.begin(); it != sl.end(); ++it ) {
		QString contact = QString(*it);
		if( contact.contains( ";" ) ) {
			uristr = contact.left( contact.find( ";" ) );
			contact = contact.mid( contact.find( ";" ) + 1 );
		} else {
			uristr = contact;
		}
		if( !uristr.isEmpty() ) {
			newcall = new SipCall( user, QString::null, SipCall::outSubscribeCall );

			printf(" \n KPHONE VIEW Constructor SIP CALL CREATION UriString = %s", uristr.ascii());
			newcall->setSubject( uristr );
      newcall->setContactStr( contact );
			SipUri remoteuri( uristr );
			member = new SipCallMember( newcall, remoteuri );
			connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
				clist, SLOT( auditList() ) );
			connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
				sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
		}
	}
	clist->auditList();
	atomId = 1000;
	p = "/kphone/" + getUserPrefix() + "/local/";
	subscribeExpiresTime = settings.readNumEntry(
		p + "/SubscribeExpiresTime", constSubscribeExpiresTime );
	if( subscribeExpiresTime == 0 ) {
		subscribeExpiresTime = -1;
	}
	subscribeTimer = new QTimer( this );
	connect( subscribeTimer, SIGNAL( timeout() ),
		this, SLOT( localStatusUpdate() ) );
	isOnline = false;
	p = "/kphone/" + getUserPrefix() + "/presence/";

	int count = 0;
	label.setNum( count );
	label = p + "/Rejected" + label;
	QString s = settings.readEntry( label, "" );
	while( !s.isEmpty() ) {
		rejectedContactList.append( s );
		label.setNum( ++count );
		label = p + "/Rejected" + label;
		s = settings.readEntry( label, "" );
	}
	cwList.setAutoDelete( true );
	imwList.setAutoDelete( true );
}

KPhoneView::~KPhoneView( void )
{
}

void KPhoneView::incomingInstantMessage( SipMessage *message )
{
        Q_UNUSED(message)
//	printf( tr("KPhoneView: Incoming Instant Message") + "\n" );
//	QString subject;
//	QString remote;
//	KInstantMessageWidget *imwidget;
//	SipCall *c;
//	subject = user->getUri().uri();
//	remote = message->getHeaderData( SipHeader::From );
//	for( imwidget = imwList.first(); imwidget != 0; imwidget = imwList.next() ) {
//		c = imwidget->getCall();
//		if( c->getSubject() == subject &&
//				remote.contains( imwidget->getRemote() ) ) {
//			imwidget->instantMessage( message );
//			imwidget->show();
//			return;
//		}
//	}
//	c = new SipCall( user );
//	c->setSubject( user->getUri().uri() );
//	imwidget = new KInstantMessageWidget( sipauthentication, c, this );
//	imwList.append( imwidget );
//	imwidget->instantMessage( message );
//	imwidget->show();
}

extern void incomingContactSubscribeCallback(QString uristr );

void KPhoneView::incomingSubscribe( SipCallMember *member, bool sendSubscribe )
{

	//clist->auditList();
	if( member == 0 ) {
		return;
	}
	printf( "KPhoneView: Incoming Subscribe\n" );
	bool remove_subscribe = false;
	SipUri uri = member->getUri();
	QString uristr = member->getUri().reqUri();

  //for ( QStringList::Iterator it = rejectedContactList.begin(); it != rejectedContactList.end(); ++it ) {
	//	if( uristr == QString(*it) ) {
	//		printf( "KPhoneView: Incoming Subscribe Rejected\n" );
	//		remove_subscribe = true;
	//	}
	//}

  // If already present in subscribe list then ignore our response
  // has already gone and the uri figures in the contactlist
  SipCallIterator it( client->getCallList() );
	bool find = false;
	if( !remove_subscribe ) {
		for( it.toFirst(); it.current(); ++it ) {
			if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
				if( it.current()->getMember( uri ) ) {
					find = true;
				}
			}
		}
	}


  // If it is not present in the outsubscribecalls then
  // send it up to the user application to take an action.
  // if user adds has already gone and the uri figures in the contactlist
  if( !find) {
		QString uristr = member->getUri().reqUri();
    // Send Call back to Adaptation Layer
    if(pVoipUserAgent)
		pVoipUserAgent->incomingContactSubscribeCallback(uristr);

  }

	if( isOnline ) {
		connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
			sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
		sendNotify( ONLINE, member );
	}

	for( it.toFirst(); it.current(); ++it ) {
		if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
			if( it.current()->getCallStatus() != SipCall::callDead ) {
				if( it.current()->getMember( uri ) ) {
					if( sendSubscribe ) {
						if( reg->getRegisterState() == SipRegister::Connected ) {
							QString uristr = it.current()->getSubject();
							QString contactStr = it.current()->getContactStr();
							if( it.current()->getCallStatus() == SipCall::callInProgress ) {
								it.current()->getMember( uri )->requestClearSubscribe();
							}
							delete it.current();
							SipCall *newcall = new SipCall( user, QString::null,
								 SipCall::outSubscribeCall );
							newcall->setSubject( uristr );
							SipUri remoteuri( uristr );
							member = new SipCallMember( newcall, remoteuri );
							connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
								clist, SLOT( auditList() ) );
							connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
								sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
							member->requestSubscribe( subscribeExpiresTime );
							newcall->setContactStr( contactStr );
							break;
						}
					}
				}
			}
		}
	}

}


// VoIP Callback for incoming call
extern void incomingCallindicationCallback(KCallWidget *);

void KPhoneView::incomingCall( SipCall *call, QString body )
{
	updateWidgetList();
	if( call->getSubject() == QString::null ) {
		call->setSdpMessageMask( body );
		call->setSubject( tr("Incoming call") );
		callAudio = new CallAudio( this );
		callAudio->readAudioSettings();
		callAudio->readVideoSettings();
		KCallWidget *widget = new KCallWidget( 0, callAudio, call, this );
		cwList.append( widget );
		connect( widget, SIGNAL( redirectCall( const SipUri &, const QString & ) ),
			this, SLOT( redirectCall( const SipUri &, const QString & ) ) );
		widget->show();
    if(pVoipUserAgent)
		pVoipUserAgent->incomingCallindicationCallback(widget);
	}

}

void KPhoneView::hideCallWidget( SipCall *call )
{
	KCallWidget *widget;
	SipCall *c;
	for( widget = cwList.first(); widget != 0; widget = cwList.next() ) {
		c = widget->getCall();
		if( c == call ) {
			widget->setHide();
		}
	}
}

void KPhoneView::contactDoubleClicked( QListViewItem *i )
{
    Q_UNUSED(i)
//	if( !buttonSipUri->text().compare( tr("Set Identity") ) ){
//		QMessageBox::information( this, tr("KPhone"), tr("Set First Identity.") );
//		return;
//	}
//  printf(" in contactDoubleClicked\n")       ;
//  QString subject = ( (ContactsListViewItem *) i )->getCall()->getSubject();
//	SipCall *newcall = new SipCall( user, QString::null, SipCall::StandardCall );
//	newcall->setSubject( user->getUri().uri() );
//	KCallWidget *widget = new KCallWidget( sipauthentication, callAudio, newcall, this );
//	cwList.append( widget );
//	connect( widget, SIGNAL( redirectCall( const SipUri &, const QString & ) ),
//		this, SLOT( redirectCall( const SipUri &, const QString & ) ) );
//	widget->pleaseDial(subject);
//	widget->show();
//  printf(" out contactDoubleClicked\n")     ;
//
}

void KPhoneView::contactSendMessage()
{
//	QString subject = "";
//	if( clist->currentItem() != 0 ) {
//		subject = ((ContactsListViewItem *)clist->currentItem())->getCall()->getSubject();
//	}
//	if( !buttonSipUri->text().compare( tr("Set Identity") ) ){
//		QMessageBox::information( this, tr("KPhone"), tr("Set First Identity.") );
//		return;
//	}
//	SipUri uri = user->getUri();
//	KInstantMessageWidget *imwidget;
//	if( clist->currentItem() ) {
//		QString subject = ( (ContactsListViewItem *)clist->currentItem() )->getCall()->getSubject();
//		if( clist->currentItem() != 0 ) {
//			for( imwidget = imwList.first(); imwidget != 0; imwidget = imwList.next() ) {
//				if( imwidget->getCall()->getSubject() == uri.uri() &&
//				    imwidget->getRemote() == subject ) {
//					imwidget->show();
//					return;
//				}
//			}
//		}
//	}
//	SipCall *initcall = new SipCall( user );
//	initcall->setSubject( uri.uri() );
//	imwidget = new KInstantMessageWidget( sipauthentication, initcall, this );
//	imwList.append( imwidget );
//	imwidget->setRemote( subject );
//	imwidget->show();
}

void KPhoneView::contactCall()
{
//  printf(" in KPhoneView::contactCall\n")     ;
//	updateWidgetList();
//	QString subject = "";
//	if( clist->currentItem() != 0 ) {
//		subject = ((ContactsListViewItem *)clist->currentItem())->getCall()->getSubject();
//	}
//	if( !buttonSipUri->text().compare( tr("Set Identity") ) ){
//		QMessageBox::information( this, tr("KPhone"), tr("Set First Identity.") );
//		return;
//	}
//	SipCall *newcall = new SipCall( user, QString::null, SipCall::StandardCall );
//	newcall->setSubject( user->getUri().uri() );
//	callAudio = new CallAudio( this );
//	callAudio->readAudioSettings();
//	callAudio->readVideoSettings();
//	KCallWidget *widget = new KCallWidget( sipauthentication, callAudio, newcall, this );
//	cwList.append( widget );
//	connect( widget, SIGNAL( redirectCall( const SipUri &, const QString & ) ),
//		this, SLOT( redirectCall( const SipUri &, const QString & ) ) );
//	widget->pleaseDial(subject);
//	widget->show();
//  printf(" out KPhoneView::contactCall\n")     ;
}

void KPhoneView::contactVideoCall()
{
//	updateWidgetList();
//	QString subject = "";
//	if( clist->currentItem() != 0 ) {
//		subject = ((ContactsListViewItem *)clist->currentItem())->getCall()->getSubject();
//	}
//	if( !buttonSipUri->text().compare( tr("Set Identity") ) ){
//		QMessageBox::information( this, tr("KPhone"), tr("Set First Identity.") );
//		return;
//	}
//	SipCall *newcall = new SipCall( user, QString::null, SipCall::videoCall );
//	newcall->setSubject( user->getUri().uri() );
//	callAudio = new CallAudio( this );
//	callAudio->readAudioSettings();
//	callAudio->readVideoSettings();
//	KCallWidget *widget = new KCallWidget( sipauthentication, callAudio, newcall, this );
//	cwList.append( widget );
//	widget->setRemote( subject );
//	connect( widget, SIGNAL( redirectCall( const SipUri &, const QString & ) ),
//		this, SLOT( redirectCall( const SipUri &, const QString & ) ) );
//	widget->show();
}


void KPhoneView::showPhoneBook()
{
//	QSettings settings;
//	QString p = "/kphone/" + getUserPrefix() + "/local/";
//	QString file = settings.readEntry( p + "/PhoneBook", "" );
//	if( file.isEmpty() ) {
//		if( getUserPrefix().isEmpty() ) {
//			file = QDir::homeDirPath() + "/.kphone-phonebook.xml";
//		} else {
//			file = QDir::homeDirPath() + "/.kphone" +
//				getUserPrefix() + "phonebook.xml";
//		}
//	}
//	QList<IncomingCall> r;
//	QList<IncomingCall> m;
//	PhoneBook *phoneBook = new PhoneBook(file, this, getUserPrefix() +
//		"Phone Book", r, m, rejectedContactList );
//	phoneBook->exec();
//	rejectedContactList = phoneBook->getRejectedContactsList();
//	saveRejectContactList();
//	updateContacts( file );
}

void KPhoneView::addContactToPhoneBook( SipCallMember *member )
{
    Q_UNUSED(member)
//	QSettings settings;
//	QString p = "/kphone/" + getUserPrefix() + "/local/";
//	QString file = settings.readEntry( p + "/PhoneBook", "" );
//	if( file.isEmpty() ) {
//		if( getUserPrefix().isEmpty() ) {
//			file = QDir::homeDirPath() + "/.kphone-phonebook.xml";
//		} else {
//			file = QDir::homeDirPath() + "/.kphone" +
//				getUserPrefix() + "phonebook.xml";
//		}
//	}
//	QList<IncomingCall> r;
//	QList<IncomingCall> m;
//	QStringList s;
//	PhoneBook *phoneBook = new PhoneBook(
//		file, this, getUserPrefix() + tr("Phone Book"), r, m, s );
//	phoneBook->addContact( member );
//	updateContacts( file );
}

void KPhoneView::updateSubscribes( void )
{
//	QSettings settings;
//	QString p = "/kphone/" + getUserPrefix() + "/local";
//	QString file = settings.readEntry( p + "/PhoneBook", "" );
//	if( file.isEmpty() ) {
//		if( getUserPrefix().isEmpty() ) {
//			file = QDir::homeDirPath() + "/.kphone-phonebook.xml";
//		} else {
//			file = QDir::homeDirPath() + "/.kphone" +
//				getUserPrefix() + "phonebook.xml";
//		}
//	}
//	updateContacts( file );
}

void KPhoneView::updateContacts( QString file )
{
    Q_UNUSED(file)
//	QString uristr;
//	SipCallMember *member;
//	SipCall *newcall;
//	QFile xmlFile( file );
//	QXmlInputSource source( xmlFile );
//	QXmlSimpleReader reader;
//	ContactParser parser;
//	reader.setContentHandler( &parser );
//	reader.parse( source );
//	SipCallIterator it( client->getCallList() );
//	QStringList::Iterator itStr;
//	QStringList sl = parser.getListContacts();
//	bool find;
//	it.toFirst();
//	while ( it.current() ) {
//		find = true;
//		if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
//			find = false;
//			for ( itStr = sl.begin(); itStr != sl.end(); ++itStr ) {
//				QString subject = QString(*itStr);
//				if( subject.contains( ";" ) ) {
//					subject = subject.left( subject.find( ";" ) );
//				}
//				if( subject == it.current()->getSubject() ) {
//					find = true;
//					break;
//				}
//			}
//			if( !find ) {
//				disconnect( it.current(), 0, 0, 0 );
//				delete it.current();
//			}
//		}
//		if( find ) {
//			++it;
//		}
//	}
//	for ( QStringList::Iterator itStr = sl.begin(); itStr != sl.end(); ++itStr ) {
//		uristr = QString(*itStr);
//		if( !uristr.isEmpty() ) {
//			find = false;
//			QString subject = QString(*itStr);
//			QString contactStr = "";
//			if( subject.contains( ";" ) ) {
//				contactStr = subject.mid( subject.find( ";" ) + 1 );
//				subject = subject.left( subject.find( ";" ) );
//			}
//			for ( it.toFirst(); it.current(); ++it ) {
//				if( subject == it.current()->getSubject() &&
//				    it.current()->getCallStatus() != SipCall::callDead ) {
//					if( !contactStr.isEmpty() ) {
//						it.current()->setContactStr( contactStr );
//					} else {
//						it.current()->setContactStr( subject );
//					}
//					find = true;
//					break;
//				}
//			}
//			if( !find ) {
//				newcall = new SipCall( user, QString::null, SipCall::outSubscribeCall );
//				newcall->setSubject( uristr );
//				SipUri remoteuri( uristr );
//				member = new SipCallMember( newcall, remoteuri );
//				connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
//					clist, SLOT( auditList() ) );
//				connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
//					sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
//				member->requestSubscribe( subscribeExpiresTime );
//				if( !contactStr.isEmpty() ) {
//					newcall->setContactStr( contactStr );
//				} else {
//					newcall->setContactStr( subject );
//				}
//			}
//		}
//	}
//	clist->auditList();
}



void KPhoneView::updateVoIPContactList( QStringList sl )
{
	QString uristr;
	SipCallMember *member;
	SipCall *newcall;
	SipCallIterator it( client->getCallList() );
	QStringList::Iterator itStr;

  bool find;
	it.toFirst();
	while ( it.current() ) {
		find = true;
		if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
			find = false;
			for ( itStr = sl.begin(); itStr != sl.end(); ++itStr ) {
				QString subject = QString(*itStr);
				if( subject.contains( ";" ) ) {
					subject = subject.left( subject.find( ";" ) );
				}
				if( subject == it.current()->getSubject() ) {
					find = true;
					break;
				}
			}
			if( !find ) {
				disconnect( it.current(), 0, 0, 0 );
				delete it.current();
			}
		}
		if( find ) {
			++it;
		}
	}
	for ( QStringList::Iterator itStr = sl.begin(); itStr != sl.end(); ++itStr ) {
		uristr = QString(*itStr);
    printf(" \n updateVoIPContactList Received UriString from Main.CPP = %s", uristr.ascii());
		if( !uristr.isEmpty() ) {
			find = false;
			QString subject = QString(*itStr);
			QString contactStr = "";
			if( subject.contains( ";" ) ) {
				contactStr = subject.mid( subject.find( ";" ) + 1 );
				subject = subject.left( subject.find( ";" ) );
			}
			for ( it.toFirst(); it.current(); ++it ) {
				if( subject == it.current()->getSubject() &&
				    it.current()->getCallStatus() != SipCall::callDead ) {
					if( !contactStr.isEmpty() ) {
						it.current()->setContactStr( contactStr );
					} else {
						it.current()->setContactStr( subject );
					}
					find = true;
					break;
				}
			}
			if( !find ) {
				newcall = new SipCall( user, QString::null, SipCall::outSubscribeCall );
  			printf(" \n updateVoIPContactList Constructor SIP CALL CREATION UriString = %s", uristr.ascii());
				newcall->setSubject( uristr );
				SipUri remoteuri( uristr );
				member = new SipCallMember( newcall, remoteuri );
			//	connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
			//		clist, SLOT( auditList() ) );
				connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
					sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
				member->requestSubscribe( subscribeExpiresTime );
				if( !contactStr.isEmpty() ) {
					newcall->setContactStr( contactStr );
				} else {
					newcall->setContactStr( subject );
				}
			}
		}
	}
	//clist->auditList();
}


void KPhoneView::localStatusUpdate( void )
{
	QString subject;
	QIconSet icon;
	SipCallIterator it( client->getCallList() );
	if( reg == 0 ) {
		return;
	}
	if( setSubscribeOffline && reg->getRegisterState() == SipRegister::Connected ) {
		setSubscribeOffline = false;
		isOnline = false;
		QIconSet icon;
		icon.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
		buttonOffOnline->setIconSet( icon );
		buttonUpdate->setEnabled( false );
		buttonOffOnline->setEnabled( false );
		it.toFirst();
		for( it.toFirst(); it.current(); ++it ) {
			if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
				if( it.current()->getCallStatus() == SipCall::callInProgress ) {
					SipCallMember *member = it.current()->getMemberList().toFirst();
					if( member ) {
						member->requestClearSubscribe();
					}
				}
			}
		}
	} else {
		if( reg->getRegisterState() == SipRegister::NotConnected ) {
			isOnline = false;
			QIconSet icon1;
			icon1.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
			buttonOffOnline->setIconSet( icon1 );
			buttonUpdate->setEnabled( false );
			buttonOffOnline->setEnabled( false );
		} else if( reg->getRegisterState() == SipRegister::Connected ) {
			if( buttonSetOffline ) {
				buttonOffOnline->setEnabled( true );
			} else {
				isOnline = true;
				QIconSet icon2;
				icon2.setPixmap(SHARE_DIR "/icons/online.png", QIconSet::Automatic );
				buttonOffOnline->setIconSet( icon2 );
				buttonUpdate->setEnabled( true );
				buttonOffOnline->setEnabled( true );
				it.toFirst();
				for( it.toFirst(); it.current(); ++it ) {
					if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
						if( it.current()->getCallStatus() != SipCall::callDead ) {
							SipCallMember *member = it.current()->getMemberList().toFirst();
							if( member ) {
								member->requestSubscribe( subscribeExpiresTime );
							}
						}
					}
				}
			}
		}
	}
	emit ( stateChanged() );
}


void KPhoneView::ChangeAvailabilityStatus( int AvailState)
{
    if (AvailState == 0) {
        stateComboBox->setCurrentItem(1);
    } else {
        stateComboBox->setCurrentItem(0);
    }

    if (isOnline)
        stateUpdated( ONLINE );
}

int KPhoneView::GetAvailabilityStatus()
{
    if ( stateComboBox->currentText().lower() == "busy" )
	return 0;
    else
	return 1;
}

void KPhoneView::buttonOffOnlineClicked( void )
{
//	if( isOnline ) {
//		buttonSetOffline = true;
//		isOnline = false;
//		buttonUpdate->setEnabled( false );
//		stateComboBox->setEnabled( false );
//		QIconSet icon1;
//		icon1.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
//		buttonOffOnline->setIconSet( icon1 );
//		stateUpdated( OFFLINE );
//	} else {
//		buttonSetOffline = false;
//		isOnline = true;
//		buttonUpdate->setEnabled( true );
//		stateComboBox->setEnabled( true );
//		QIconSet icon2;
//		icon2.setPixmap(SHARE_DIR "/icons/online.png", QIconSet::Automatic );
//		buttonOffOnline->setIconSet( icon2 );
//		stateUpdated( ONLINE );
//	}
}

void KPhoneView::buttonUpdateClicked( void )
{
//	if( isOnline ) {
//		stateUpdated( ONLINE );
//	}
}


//Added
void changePresenceStatusCallback(QString uristr, int presenceState );


void KPhoneView::presenceStatusListUpdation( void )
{
	bool isSubscribed = false;
	//clear();
 	SipCallIterator it( client->getCallList() );
	SipCallIterator it2( client->getCallList() );
	it.toFirst();
	for( it.toFirst(); it.current(); ++it ) {
		if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
			isSubscribed = false;
			it2.toFirst();
			for( it2.toFirst(); it2.current(); ++it2 ) {
				if( it2.current()->getCallType() == SipCall::inSubscribeCall ) {
					if( it2.current()->getMember( it.current()->getSubject() ) ) {
						if( it2.current()->getCallStatus() == SipCall::callInProgress ) {
							isSubscribed = true;
						}
					}
				}
			}
			if( it.current()->getCallStatus() != SipCall::callDead ) {
                            QString UriStr = it.current()->getSubject() ;
                            qDebug(" presence status List updation UriString = %s", UriStr.ascii());

                            bool isPresent = true;
                            if (/*!isSubscribed ||*/ it.current()->getPresenceStatus() == "Busy")
                                isPresent = false;

                            if( pVoipUserAgent)
                                pVoipUserAgent->changePresenceStatusCallback(UriStr, isPresent);
                        }
                }
        }
}
void KPhoneView::stateUpdated( int id )
{
	QString subject;
	if( id == OFFLINE ) {
		isOnline = false;
		buttonUpdate->setEnabled( false );
		QIconSet icon;
		icon.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
		buttonOffOnline->setIconSet( icon );
	}
	SipCallIterator it( client->getCallList() );
	it.toFirst();
	for( it.toFirst(); it.current(); ++it ) {
		if( it.current()->getCallType() == SipCall::inSubscribeCall ) {
			if( it.current()->getCallStatus() == SipCall::callInProgress ) {
				SipCallMember *member = it.current()->getMemberList().toFirst();
				if( member ) {
					sendNotify( id, member );
				}
			} else {
				delete it.current();
			}
		}
	}
	emit( stateChanged() );
	
}

QString KPhoneView::stateText( QString text )
{
	QString state;
	if( text == tr("Online") ) {
		state = "online";
	} else if( text == tr("Busy") ) {
		state = "busy";
	} else if( text == tr("Be Right Back") ) {
		state = "berightback";
	} else if( text == tr("Away") ) {
		state = "away";
	} else if( text == tr("On The Phone") ) {
		state = "onthephone";
	} else if( text == tr("Out To Lunch") ) {
		state = "outtolunch";
	} else {
		state = text;
	}
	return state;
}


void KPhoneView::sendNotify( int id, SipCallMember *member )
{
	QString s;
	QString tmp;
	s =  "<?xml version=\"1.0\"?>\n";
	s += "<!DOCTYPE presence\n";
	s += "PUBLIC \"-//IETF//DTD RFCxxxx XPIDF 1.0//EN\" \"xpidf.dtd\">\n";

	s += "<presence>\n";
	s += "<presentity uri=\"";
	s += member->getUri().reqUri();
	s += ";method=SUBSCRIBE\"/>\n";
	s += "<atom id=\"";
	s += tmp.setNum( atomId++ ).latin1();
	s += "\">\n";
	s += "<address uri=\"";
	s += client->getContactUri().reqUri();
	s += ";user=ip\" priority=\"0,800000\">\n";
	s += "<status status=\"open\"/>\n";
	if( id == ONLINE ) {
		if( stateComboBox->currentText().isEmpty() ) {
			s += "<msnsubstatus substatus=\"online\"/>\n";
		} else {
			s += "<msnsubstatus substatus=\"";
			s += stateText( stateComboBox->currentText() );
			s += "\" />\n";
		}
	} else {  
		s += "<msnsubstatus substatus=\"\"/>\n";
	}
	s += "</address>\n";
	s += "</atom>\n";
	s += "</presence>\n";
	if( stateComboBox->currentText().lower() == "busy" ) {
		client->setBusy( true );
	} else {
		client->setBusy( false );
	}
	if( member != 0 ) {
		if( id == OFFLINE ) {
			member->requestClearNotify( s.latin1(), MimeContentType( "application/xpidf+xml" ) );
		} else {
			if( !buttonSetOffline ) {
				member->requestNotify( s.latin1(), MimeContentType( "application/xpidf+xml" ) );
			}
		}
	}
}

void KPhoneView::redirectCall( const SipUri &calluri, const QString &subject )
{
  printf(" in KPhoneView::redirectCall\n")     ;
	SipCall *newcall = new SipCall( user );
	newcall->setSubject( subject );
	callAudio = new CallAudio( this );
	callAudio->readAudioSettings();
	callAudio->readVideoSettings();
	KCallWidget *widget = new KCallWidget( sipauthentication, callAudio, newcall, this );
	cwList.append( widget );
	widget->pleaseDial( calluri );
	connect( widget, SIGNAL( redirectCall( const SipUri &, const QString & ) ),
		this, SLOT( redirectCall( const SipUri &, const QString & ) ) );
	widget->show();
  printf(" out KPhoneView::redirectCall\n")     ;
}

KCallWidget *KPhoneView::DoCall( QString num, SipCall::CallType ctype )
{
	updateWidgetList();
	if( !buttonSipUri->text().compare( tr("Set Identity") ) ){
    //PD QMessageBox::information( this, tr("KPhone"), tr("Set First Identity.") );
		return NULL;
	}
	SipCall *newcall = new SipCall( user, QString::null, ctype );
	QString subject;
	subject.sprintf( user->getUri().uri() );
	newcall->setSubject( subject );
	callAudio = new CallAudio( this );
	callAudio->readAudioSettings();
	callAudio->readVideoSettings();
	KCallWidget *widget = new KCallWidget( sipauthentication, callAudio, newcall, this );
	cwList.append( widget );
	widget->setRemote( num );
	if( !num.isEmpty() ) {
		widget->clickDial();
	}
	connect( widget, SIGNAL( redirectCall( const SipUri &, const QString & ) ),
		this, SLOT( redirectCall( const SipUri &, const QString & ) ) );
	widget->show();
	
	return widget;
}



// MakeVoIPCall API
//
KCallWidget *KPhoneView::VoIPCallDial( QString num, SipCall::CallType ctype )
{
  //updateWidgetList();
	SipCall *newcall = new SipCall( user, QString::null, ctype );
	QString subject;
	subject.sprintf( user->getUri().uri() );
	newcall->setSubject( subject );
	callAudio = new CallAudio( this );
	callAudio->readAudioSettings();
	callAudio->readVideoSettings();
	KCallWidget *widget = new KCallWidget( sipauthentication, callAudio, newcall, this );
	cwList.append( widget );
	widget->setRemote( num );
	//if( !num.isEmpty() ) {
	widget->clickDial();
	//}
	//connect( widget, SIGNAL( redirectCall( const SipUri &, const QString & ) ),
	//	this, SLOT( redirectCall( const SipUri &, const QString & ) ) );
	//widget->show();

	return widget;
}

void KPhoneView::makeNewCall( void )
{
//	if (DoCall(lineeditCall->text(), SipCall::StandardCall))
//		lineeditCall->setText("");
}

void KPhoneView::makeVideoCall( void )
{
//	if (DoCall(lineeditCall->text(), SipCall::videoCall))
//		lineeditCall->setText("");
}

void KPhoneView::updateIdentity( SipUser *newUser, SipRegister *newReg )
{
	if( newReg != 0 ) {
		if( reg != 0 ) {
			disconnect( reg, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdate() ) );
		}
		reg = newReg;
		connect( reg, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdate() ) );
		localStatusUpdate();
	}
	buttonSipUri->setText( newUser->getUri().uri() );
	user = newUser;
}

void KPhoneView::busyToggled( bool onoff )
{
	client->setBusy( onoff );
}

void KPhoneView::showIdentities( void )
{
//	identitiesDialog->showIdentity();
}

void KPhoneView::identities( KSipRegistrations *i )
{
	identitiesDialog = i;
}

void KPhoneView::setContactsOffline( void )
{
	stateUpdated( KPhoneView::OFFLINE );
	setSubscribeOffline = true;
	localStatusUpdate();
}

void KPhoneView::setContactsOnline( void )
{
	stateUpdated( KPhoneView::ONLINE );
}

void KPhoneView::setStunSrv( QString stunuri )
{
    Q_UNUSED(stunuri)
//	if( !stunuri.isEmpty() ) {
//		useStunProxy = true;
//		stunProxyUri = stunuri;
//	}
}

bool KPhoneView::useStunSrv( void )
{
	return useStunProxy;
}

QString KPhoneView::getStunSrv( void )
{
	return stunProxyUri;
}


void KPhoneView::kphoneQuit( void )
{
	KCallWidget *widget;
	for( widget = cwList.first(); widget != 0; widget = cwList.next() ) {
		if( widget->isHided() ) {
			cwList.remove( widget );
		} else {
			widget->clickHangup();
		}
	}
	setContactsOffline();
	identitiesDialog->unregAllRegistration();
}

QString KPhoneView::getUserPrefix( void )
{
	return ((KPhone *)parentWidget())->getUserPrefix();
}

void KPhoneView::saveRejectContactList( void )
{
//	QSettings settings;
//	QString p = "/kphone/" + getUserPrefix() + "/presence/";
//	QString label;
//	int count = 0;
//	label.setNum( count );
//	label = p + "/Rejected" + label;
//	QString s = settings.readEntry( label, "" );
//	while( !s.isEmpty() ) {
//		settings.writeEntry( label, "");
//		label.setNum( ++count );
//		label = p + "/Rejected" + label;
//		s = settings.readEntry( label, "" );
//	}
//	count = 0;
//	for ( QStringList::Iterator it = rejectedContactList.begin(); it != rejectedContactList.end(); ++it ) {
//		label.setNum( count++ );
//		label = p + "/Rejected" + label;
//		settings.writeEntry( label, QString(*it) );
//	}
}

void KPhoneView::updateWidgetList( void )
{
	KCallWidget *widget;
	for( widget = cwList.first(); widget != 0; widget = cwList.next() ) {
		if( widget->isHided() ) {
			cwList.remove( widget );
		}
	}
}

bool KPhoneView::getState( void )
{
	return isOnline;
}

