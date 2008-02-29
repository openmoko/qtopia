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
#ifndef KPHONE_H_INCLUDED
#define KPHONE_H_INCLUDED

#include <qmainwindow.h>

class SipClient;
class SipUser;
class PhoneList;
class CallAudio;
class AudioControl;
class VideoControl;
class KSipUserInfo;
class KPhoneView;
class KSipRegistrations;
class KSipAuthentication;
class KSipPreferences;
class KSipOptions;
class SipRegister;
class KTestOptions;

QString const KPhoneVersion = "4.1.1";
int const quitTime = 5000;
int const ringTime_1 = 200;
int const ringTime_2 = 2000;
int const acceptTime = 500;
int const acceptSubscribeSendToSubscribe_time = 5000;
int const constRegistrationExpiresTime = 900;
int const constSubscribeExpiresTime = 600;
int const constStunRequestPeriod = 60;
int const constMinPort = 0;
int const constMaxPort = 0;
int const kphoneMinimunWidht = 220;
QString const constStunServer = "stun.wirlab.net:3478";


class KPhone : public QMainWindow
{
	Q_OBJECT
public:
	KPhone( unsigned int listenport, QString prefix, bool looseRoute, bool strictRoute );
	~KPhone( void );
	void showUsers( void );
	QString getUserPrefix( void );
	KPhoneView *getView( void );
	KSipRegistrations *getSipRegistrations( void ) {return  registrations;};
	KSipPreferences *getSipPreferences( void ){return  sipprefs;};
	KSipAuthentication *getSipAuthentication( void ){return  authentication;};
	KSipOptions *getSipOptions( void ){return  queryoptions;};
	SipClient *getSipclient( void ){return  client;};
  
public slots:
	void showHelp( void );
	void toggleVisibility( void );
	void trayMenuRequested( const QPoint& pos );

private slots:
	void timerTick( void );
	void stun_timerTick( void );
	void showUserInfo( void );
	void showAudioPreferences( void );
	void showVideoPreferences( void );
	void showRegistrations( void );
	void showGlobalPreferences( void );
	void showQueryOptions( void );
	void showTestOptions( void );
	void showAboutKPhone( void );
	void kphoneQuit( void );

protected:
	void closeEvent( QCloseEvent* );
	KPhoneView *view;

	// Global state
	SipClient *client;
	SipUser *user;
	CallAudio *audio;
	QPopupMenu *preferencesMenu;
public:
	// Dialogs
	PhoneList *qdial;
	KSipUserInfo *userinfo;
	AudioControl *audiocontrol;
	VideoControl *videocontrol;
	KSipRegistrations *registrations;
	KSipAuthentication *authentication;
	KSipPreferences *sipprefs;
	KSipOptions *queryoptions;
	KTestOptions *testoptions;

private:
	QTimer *quitTimer;
	QString userPrefix;

};

#endif // KPHONE_H_INCLUDED
