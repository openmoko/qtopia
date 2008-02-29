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
#ifndef KSIPREGISTRATIONS_H_INCLUDED
#define KSIPREGISTRATIONS_H_INCLUDED
#include <qdialog.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qbutton.h>
#include <qlabel.h>

#include "../dissipate2/sipcall.h"


class QLineEdit;
class SipUser;
class SipClient;
class SipRegister;
class KPhoneView;

struct Kstate {
  public:
        enum KsipState {
            OFFLINE    = 0,
            UNREG      = 1,
            REG        = 2, 
            AUTHREQ    = 3,
            PROC_UNREG = 10,
            PROC_REG   = 11,
            PROC_TRY   = 12,
            LAST = 99
        };
};

class KSipIdentityEdit : public QDialog , public Kstate
{
	Q_OBJECT
public:
	KSipIdentityEdit( QWidget *parent, const char *name,  QObject *reg );
	~KSipIdentityEdit( void );
	void setReadOnly( bool mode );
	void updateState( KsipState state );
	void setAutoRegister( bool newAutoRegister ) { autoRegister->setChecked( newAutoRegister ); }
	bool getAutoRegister( void ) { return autoRegister->isChecked(); }
	QString getFullname( void ) const;
	QString getUsername( void ) const;
	QString getHostname( void ) const;
	QString getSipProxy( void ) const;
	QString getSipProxyUsername( void ) const;
	QString getUri( void ) const;
	QString getQvalue( void ) const;
	void setFullname( const QString &newFullname );
	void setUsername( const QString &newUsername );
	void setHostname( const QString &newHostname );
	void setSipProxy( const QString &newSipProxy );
	void setSipProxyUsername( const QString &newSipProxyUsername );
	void setDefault( bool newDefault );
	void setQvalue( const QString &newSipProxy );

signals:
	void update( void );

protected slots:
	void slotOk( void );
	void slotCancel( void );

private:
	QLineEdit *fullname;
	QLineEdit *username;
	QLineEdit *hostname;
	QLineEdit *sipProxy;
	QLineEdit *sipProxyUsername;
	QLineEdit *qValue;
	QCheckBox *autoRegister;
	QLabel *labelRegister;
	QPushButton *buttonRegister;
	bool editMode;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

class KSipRegistrations : public QObject , public Kstate
{
	Q_OBJECT
public:
	KSipRegistrations( SipClient *client,
		KPhoneView *phoneView = 0, QWidget *p = 0, const char *name = 0 );
	~KSipRegistrations( void );
	void showIdentity( void );
	void unregAllRegistration( void );
	bool getUseStun( void ) { return useStun; }
	QString getStunSrv( void ) { return stunSrv; }
  int RegisterVoIPSip(const QString &username, const QString &password);

public slots:
        void update( void );

private slots:
	void changeRegistration( void );
	void editRegistration( void );
	void registerStatusUpdated( void );

private:
	void setRegisterState( void );
	void save( void );
	SipClient *c;

  #ifdef KPHONE_APP
	KSipIdentityEdit *edit;
  #endif
  KPhoneView *v;
	QString userPrefix;
	SipUser *u;
	SipRegister *sipreg;
	bool autoRegister;
	int expires;
	QWidget *parent;
	bool useStun;
	QString stunSrv;
};

#endif // KSIPREGISTRATIONS_H_INCLUDED
