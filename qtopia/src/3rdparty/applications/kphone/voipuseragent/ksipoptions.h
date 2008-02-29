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
#ifndef KSIPOPTIONS_H_INCLUDED
#define KSIPOPTIONS_H_INCLUDED

#include <qdialog.h>

#include "../dissipate2/sipclient.h"

class SipUser;
class SipCall;
class SipCallMember;
class QLineEdit;
class QMultiLineEdit;

class KSipOptions : public QDialog
{
	Q_OBJECT
public:
	KSipOptions( SipUser *user, QWidget *parent = 0, const char *name = 0 );
	~KSipOptions( void );

private slots:
	void queryOptions( void );
	void optionsCallStatusUpdated( void );

private:
	SipUser *u;
	SipCall *call;
	SipCallMember *member;
	QLineEdit *queryuri;
	QMultiLineEdit *qresults;
};

class KTestOptions : public QDialog
{
	Q_OBJECT
public:
	KTestOptions( SipClient *client );
	~KTestOptions( void );

public slots:
	void testSendMessage( void );

protected slots:
	void slotOk( void );

private slots:
	void testClean( void );
	void testFile( void );
	void testSend( void );

private:
	SipClient *c;
	QLineEdit *addr;
	QLineEdit *port;
	QMultiLineEdit *msg;
	QString testStr;
};


#endif // KSIPOPTIONS_H_INCLUDED
