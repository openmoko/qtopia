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
#ifndef KCALLWIDGET_H_INCLUDED
#define KCALLWIDGET_H_INCLUDED

#include <qdialog.h>
#include <qptrdict.h>
#include <qevent.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <time.h>

#include "../dissipate2/sipuri.h"
#include "kphonebook.h"



class QLabel;
class QLineEdit;
class KComboBox;
class QPushButton;
class QTextView;
class QMultiLineEdit;
class SipUser;
class SipCall;
class SipCallMember;
class SipTransaction;
class CallAudio;
class IncomingCall;
class SipMessage;
class KSipAuthentication;
class KPhoneView;

class KCallTransfer : public QDialog
{
	Q_OBJECT
public:
	KCallTransfer( QWidget *parent = 0, const char *name = 0 );
	~KCallTransfer( void );

signals:
	void pleaseTransfer( const QString & );

protected slots:
	void slotOk( void );
	void slotCancel( void );

private:
	QLineEdit *touri;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

class KPhoneView;
class KCallWidget  : public QObject
{
	Q_OBJECT
public:
	KCallWidget( KSipAuthentication *auth, 
			CallAudio *callaudio, 
			SipCall *initcall,
			KPhoneView *parent,
			const char *name = 0 );
	~KCallWidget( void );
	SipCall *getCall();
	void switchCall( SipCall *newcall );
	void setRemote( QString newremote );
	void clickDial( void );
	void clickHangup( void );
	void setHide( void );
	bool isHided( void ) const { return hided; }
	SipCallMember * getCallMember(void) const { return member;}
  void KCallWidget::show( void );

        void feedTone(const QString& tones);

public slots:
	void pleaseDial( const SipUri &dialuri );

signals:
	void callDeleted( void );
	void redirectCall( const SipUri &calluri, const QString &subject );

// PD  
public slots:
	void hangupCall( void );
	void acceptCall( void );
	void holdCall( void );

private slots:
	void dialClicked( void );
	void audioOutputDead( void );
	void callMemberStatusUpdated( void );
	void updateAudioStatus( void );
	void handleRedirect( void );
	void handleRefer( void );
	void showTransferDialog( void );
	void pleaseTransfer( const QString &transferto );
	void getUri( void );
	void dtmfsenderTimeout( void );
	void ringTimeout( void );
	void acceptCallTimeout( void );
	void hideCall( void );

private:
	//QList<IncomingCall> receivedCalls;
	//QList<IncomingCall> missedCalls;
	IncomingCall *incomingCall;
	void updateCallRegister( void );
	void createKeypad(void);

	// Sip Stuff
	SipCall *call;
	SipCallMember *member;
	KPhoneView *phoneView;
	KSipAuthentication *sipauthentication;

	// Audio Stuff
	CallAudio *audio;

	int ringCount;
	QTimer *ringTimer;
	bool isRingingTone;
	QTimer *acceptCallTimer;
	int audio_fd;
	QString subject;

	// DTMF Stuff
	QTimer *dtmfsenderTimer;
        QString dtmfsender;
        const float dtmfsenderdelay;

	// GUI Stuff
	//QLabel *curstat;
	//QLabel *curaudiostat;
	//QPushButton *dial;
	//QPushButton *hangup;
	//QPushButton *holdbutton;
	//QPushButton *transferbutton;
	//QPushButton *morebutton;
	//QPushButton *hidebutton;
	//QComboBox *remote;
	//QComboBox *dtmfsender;
	//KCallTransfer *transfer;
	PhoneBook *phoneBook;
	bool hided;
	//QButtonGroup* keypad;
	//Rj
	QString uriToCall; 

  public:
	// State stuff
	enum CallState {
		PreDial,
		Calling,
		Connected,
		Called };
	CallState curstate;

  public:
    // Private functions
	void forceDisconnect( void );
	QString getUserPrefix( void );
};

class KInstantMessageWidget : public QDialog
{
	Q_OBJECT
public:
	KInstantMessageWidget( KSipAuthentication *auth,
		SipCall *initcall, QWidget *parent = 0, const char *name = 0 );
	~KInstantMessageWidget( void );
	void instantMessage( SipMessage *message );
	SipCall *getCall();
	QString getRemote();
	void setRemote( QString newremote );

private slots:
	void mClicked( void );
	void getUri( void );

private:
	QList<IncomingCall> receivedCalls;
	QList<IncomingCall> missedCalls;
	IncomingCall *incomingCall;

	// Sip Stuff
	SipCall *imcall;
	SipCallMember *member;
	KSipAuthentication *sipauthentication;

	// GUI Stuff
	QTextView *status;
	QMultiLineEdit *mEdit;
	QPushButton *mbutton;
	QPushButton *qbutton;
	QPushButton *hidebutton;
	QComboBox *remote;
	KCallTransfer *transfer;
	PhoneBook *phoneBook;

	// Private functions
	QString getUserPrefix( void );
};


#endif // KCALLWIDGET_H_INCLUDED
