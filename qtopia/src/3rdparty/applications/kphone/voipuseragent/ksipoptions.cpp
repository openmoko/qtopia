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
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qlayout.h>

#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipclient.h"
#include "ksipoptions.h"

KSipOptions::KSipOptions( SipUser *user, QWidget *parent, const char *name )
	: QDialog( parent, name, false )
{
	u = user;
	QHBox *hbox = new QHBox( this );
	hbox->setMargin( 3 );
	hbox->setSpacing( 3 );
	QHBoxLayout *hboxl = new QHBoxLayout( this, 5 );
	hboxl->addWidget( hbox );
	QVBox *lvbox = new QVBox( hbox );
	(void) new QLabel( tr("URI to Request Options For:"), lvbox );
	queryuri = new QLineEdit( lvbox );
	(void) new QLabel( tr("Results:"), lvbox );
	qresults = new QMultiLineEdit( lvbox );

	QVBox *rvbox = new QVBox( hbox );

	QPushButton *query = new QPushButton( tr("Make Query"), rvbox );
	connect( query, SIGNAL( clicked() ), this, SLOT( queryOptions() ) );

	call = 0;
}

KSipOptions::~KSipOptions( void )
{
}

void KSipOptions::queryOptions( void )
{
	if( call ) {
		delete call;
		call = 0;
	}

	if( ( queryuri->text() != QString::null ) && ( !call ) ) {
		call = new SipCall( u, QString::null, SipCall::OptionsCall );
		SipUri remoteuri( queryuri->text() );
		member = new SipCallMember( call, remoteuri );
		connect( member, SIGNAL( statusUpdated( SipCallMember * ) ), this, SLOT( optionsCallStatusUpdated() ) );
		member->requestOptions();
	}
}

void KSipOptions::optionsCallStatusUpdated( void )
{
	qresults->setText( member->getSessionDescription() );
}

KTestOptions::KTestOptions( SipClient *client )
	: QDialog( 0, 0, false )
{
	c = client;
	connect( c, SIGNAL( incomingTestMessage() ), this, SLOT( testSendMessage() ) );

	QHBox *hbox = new QHBox( this );
	hbox->setMargin( 3 );
	hbox->setSpacing( 3 );
	QHBoxLayout *hboxl = new QHBoxLayout( this, 5 );
	hboxl->addWidget( hbox );

	QVBox *lvbox = new QVBox( hbox );
	(void) new QLabel( tr("Address:"), lvbox );
	addr = new QLineEdit( lvbox );
	(void) new QLabel( tr("Port:"), lvbox );
	port = new QLineEdit( lvbox );
	(void) new QLabel( tr("Message:"), lvbox );
	msg = new QMultiLineEdit( lvbox );

	QVBox *rvbox = new QVBox( hbox );

	QPushButton *clean = new QPushButton( tr("Clean"), rvbox );
	connect( clean, SIGNAL( clicked() ), this, SLOT( testClean() ) );
	QPushButton *file = new QPushButton( tr("File"), rvbox );
	connect( file, SIGNAL( clicked() ), this, SLOT( testFile() ) );
	QPushButton *send = new QPushButton( tr("Send"), rvbox );
	connect( send, SIGNAL( clicked() ), this, SLOT( testSend() ) );

	addr->setText("127.0.0.1");
	port->setText("5060");
}

KTestOptions::~KTestOptions( void )
{
}

void KTestOptions::slotOk( void )
{
	c->setTest( false );
	QDialog::accept();
}

void KTestOptions::testClean( void )
{
	msg->setText("");
}

void KTestOptions::testFile( void )
{
#ifdef AKENNEDY
	QString s = QFileDialog::getOpenFileName(
		"",
		tr("Text files (*.txt)"),
		this,
		tr("open file dialog"
		"Choose a file") );
	QFile file( s );
	msg->setText("");
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.eof() ) {
			msg->insertLine( stream.readLine() );
		}
		file.close();
	}
#endif
}

void KTestOptions::testSend( void )
{
	testStr = msg->text();
	testSendMessage();
}

void KTestOptions::testSendMessage( void )
{
	QString m;
	if ( testStr.contains("#TEST#" ) ) {
		m = testStr.left( testStr.find( "#TEST#" ) );
		testStr = testStr.mid( testStr.find( "#TEST#" ) + 7 );
	} else {
		m = testStr;
		testStr = "";
	}
	if( !addr->text().isEmpty() && !port->text().isEmpty() && !m.isEmpty() ) {
		c->sendTestMessage( addr->text(), port->text().toUInt(), m );
	}
}
