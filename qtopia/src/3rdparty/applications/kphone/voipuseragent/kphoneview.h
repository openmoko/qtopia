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
#ifndef KPHONEVIEW_H_INCLUDED
#define KPHONEVIEW_H_INCLUDED
#include <qwidget.h>
#include <qptrdict.h>
#include <qvbox.h>
#include <qradiobutton.h>
#include <qxml.h>
#include <qcombobox.h>

#include "../dissipate2/sipuri.h"
#include "ksipauthentication.h"

class SipClient;
class SipUser;
class SipCall;
class SipCallMember;
class KCallWidget;
class KInstantMessageWidget;
class QListViewItem;
class CallAudio;
class KPhone;
class KSipRegistrations;
class KSipAuthentication;
class ContactsListView;


class ContactParser : public QXmlDefaultHandler
{
public:
	ContactParser( void );
	bool startDocument();
	bool startElement( const QString&, const QString&, const QString&, const QXmlAttributes& );
	bool endElement( const QString&, const QString&, const QString& );
	QStringList getListContacts( void ) { return contacts; }
private:
	QStringList contacts;
};

class KPhoneView : public QWidget
{
	Q_OBJECT
public:
	enum State { ONLINE, OFFLINE };
	KPhoneView( SipClient *c, SipUser *u, QWidget *parent = 0, const char *name = 0 );
	~KPhoneView( void );
	void switchCall( SipCall *call );
	void updateIdentity( SipUser *newUser, SipRegister *newReg = 0 );
	void identities( KSipRegistrations *i );
	void updateContacts( QString file );
	void setContactsOffline( void );
	void setContactsOnline( void );
	void setStunSrv( QString );
	bool useStunSrv( void );
	QString getStunSrv( void );
	void kphoneQuit( void );
	bool getState( void );
	KCallWidget *KPhoneView::DoCall( QString num, SipCall::CallType ctype );
  // VoIP APIs
  KCallWidget *VoIPCallDial( QString num, SipCall::CallType ctype );
  void updateVoIPContactList( QStringList sl );
  void ChangeAvailabilityStatus( int AvailState);
  int GetAvailabilityStatus();



signals:
	void stateChanged( void );

public slots:
	void makeNewCall( void );
	void makeVideoCall( void );
	void redirectCall( const SipUri &calluri, const QString &subject );
	void showIdentities( void );
	void localStatusUpdate( void );
	void stateUpdated( int id );
	void updateSubscribes( void );
	void buttonOffOnlineClicked( void );
  //Added by us
  void presenceStatusListUpdation( void );


private slots:
	void incomingInstantMessage( SipMessage *message );
	void incomingSubscribe( SipCallMember *member, bool sendSubscribe );
	void incomingCall( SipCall *call, QString body );
	void hideCallWidget( SipCall *call );
	void contactDoubleClicked( QListViewItem *i );
	void busyToggled( bool onoff );
	void contactSendMessage();
	void contactCall( void );
	void contactVideoCall( void );
	void showPhoneBook( void );
	void buttonUpdateClicked( void );

public:
	SipClient *client;
	SipUser *user;
	ContactsListView *clist;
	int callcount;
	QPushButton *buttonSipUri;
	QPushButton *buttonNewCall;
	QPushButton *buttonVideoCall;
	QLineEdit *lineeditCall;
	QLabel *labelSipUri;
	QLabel *labelFullname;
	KPhone *kphone;
	KSipRegistrations *identitiesDialog;
	KSipAuthentication *sipauthentication;

private:
	void sendNotify( int id, SipCallMember *member = 0 );
	QString getUserPrefix( void );
	void saveRejectContactList( void );
	QList<KCallWidget> cwList;
	QList<KInstantMessageWidget> imwList;
	QStringList rejectedContactList;
	void saveContacts( void );
	void addContactToPhoneBook( SipCallMember *member );
	SipRegister *reg;
	QPushButton *buttonOffOnline;
	QPushButton *buttonUpdate;
	QComboBox *stateComboBox;
	int atomId;
	QTimer *subscribeTimer;
	int subscribeExpiresTime;
	CallAudio *callAudio;
	bool isOnline;
	bool buttonSetOffline;
	bool setSubscribeOffline;
	void updateWidgetList( void );
	QString stateText( QString text );
	bool useStunProxy;
	QString stunProxyUri;
};

#endif // KPHONEVIEW_H_INCLUDED
