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
#ifndef KSIPAUTHENTICATION_H_INCLUDED
#define KSIPAUTHENTICATION_H_INCLUDED
#include <qdialog.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qbutton.h>
#include <qlabel.h>

#include "../dissipate2/sipcall.h"
#include "ksipregistrations.h"


class KSipAuthenticationRequest : public QDialog
{
	Q_OBJECT
public:
	KSipAuthenticationRequest( const QString &server, const QString &sipuri,
		const QString &caption, QWidget *parent = 0, const char *name = 0 );
	~KSipAuthenticationRequest( void );
	void setUsername( const QString &newUsername );
	void setPassword( const QString &newPassword );
	QString getUsername( void );
	QString getPassword( void );

private slots:
	void okClicked( void );

private:
	QCheckBox *savePassword;
	QLineEdit *username;
	QLineEdit *password;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

class KSipAuthentication : public QObject
{
	Q_OBJECT
public:
	KSipAuthentication( void );
	~KSipAuthentication( void );

public slots:
	void authRequest( SipCallMember *member );

private:
	void setRegisterState( void );
	void save( void );
	SipClient *c;
	KSipIdentityEdit *edit;
	KPhoneView *v;
	QString userPrefix;
	SipUser *u;
	SipRegister *sipreg;
	bool autoRegister;
	int expires;
	QWidget *parent;
	bool useStun;
	QString stunSrv;
	KSipAuthenticationRequest *authreq;
	bool execAuthreq;
};

#endif // KSIPAUTHENTICATION_H_INCLUDED
