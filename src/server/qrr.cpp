/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qrr.h"

#include <qtopia/qcopenvelope_qws.h>
#include <qfile.h>
#include <qtimer.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qevent.h>


class CenteringDialog : public QDialog
{
public:
    CenteringDialog( QWidget *parent = 0, char *name = 0, bool modal = FALSE, WFlags f = 0 );
    virtual ~CenteringDialog();

protected:
    void resizeEvent( QResizeEvent *e );
};

CenteringDialog::CenteringDialog( QWidget *parent, char *name, bool modal, WFlags f )
    : QDialog( parent, name, modal, f )
{
}

CenteringDialog::~CenteringDialog()
{
}

void CenteringDialog::resizeEvent( QResizeEvent *e )
{
    int dist = -((width() - e->oldSize().width()) / 2);
    qDebug( "move %d", dist );
    move( pos().x() + dist, pos().y() );
}

// =====================================================================

QueuedRequestRunner::QueuedRequestRunner( QFile *f, QWidget *parent )
    : readyToDelete( FALSE ), waitingForMessages( FALSE ), file( 0 )
{
    file = f;
    waitMsgs.setAutoDelete( TRUE );
    if ( parent ) {
	progressDialog = new CenteringDialog( parent, 0, TRUE );
	QVBoxLayout *l = new QVBoxLayout( progressDialog );
	l->setMargin( 6 );
	l->setSpacing( 6 );
	progressLabel = new QLabel( progressDialog );
	progressLabel->setText( tr("Processing Queued Requests") );
	progressBar = new QProgressBar( progressDialog );
	l->addWidget( progressLabel );
	l->addWidget( progressBar );
	//progressDialog->setFixedSize( qApp->desktop()->width(), qApp->desktop()->height() );
	progressDialog->show();
    }
    int totalSteps = countSteps();
    if ( parent ) {
	qDebug( "%d steps", totalSteps );
	progressBar->setTotalSteps( totalSteps );
	progressBar->setProgress( 0 );
    }
    file->open( IO_ReadOnly );
}

QueuedRequestRunner::~QueuedRequestRunner()
{
    delete progressDialog;
    delete file;
}

void QueuedRequestRunner::process()
{
    if ( process( FALSE ) ) {
	if ( !waitingForMessages || action == "wait" )
	    QTimer::singleShot( 100, this, SLOT(process()) );
    } else {
	file->remove();
	emit finished();
    }

}

int QueuedRequestRunner::countSteps()
{
    int totalSteps = 0;
    bool more = TRUE;
    file->open( IO_ReadOnly );
    while ( more ) {
	steps = 0;
	more = process( TRUE );
	totalSteps += steps;
    }
    file->close();
    waitingForMessages = FALSE;
    return totalSteps;
}

bool QueuedRequestRunner::process( bool counting )
{
    QDataStream stream( file );
    stream >> action;
    if ( action == "info" ) {
	QString message;
	stream >> message;
	qDebug( "info %s", message.latin1() );
	if ( counting ) {
	    steps++;
	} else {
	    progressLabel->setText( message );
	}
    } else if ( action == "qcop" ) {
	QCString channel;
	QCString message;
	int args;
	stream >> channel >> message >> args;
	qDebug( "qcop %s %s", channel.data(), message.data() );
#ifndef QT_NO_COP
	QCopEnvelope *e = 0;
	if ( !counting ) {
	    e = new QCopEnvelope( channel, message );
	}
#endif
	QCString type;
	for ( int i = 0; i < args; ++i ) {
	    stream >> type;
	    if ( type == "QString" ) {
		QString arg;
		stream >> arg;
		qDebug( "     %s %s", type.data(), arg.latin1() );
#ifndef QT_NO_COP
		if ( !counting )
		    (*e) << arg;
#endif
	    } else if ( type == "int" ) {
		int arg;
		stream >> arg;
		qDebug( "     %s %d", type.data(), arg );
#ifndef QT_NO_COP
		if ( !counting )
		    (*e) << arg;
#endif
	    } else {
		qDebug( "\tBUG unknown type '%s'!", type.data() );
	    }
	}
	if ( counting ) {
	    steps++;
	} else {
#ifndef QT_NO_COP
	    // this causes the QCop message to be sent
	    delete e;
#endif
	}
    } else if ( action == "wait" ) {
	int messageCount;
	QCString message;
	waitMsgs.clear();
	stream >> messageCount;
	for ( int i = 0; i < messageCount; ++i ) {
	    stream >> message;
	    qDebug( "wait %s", message.data() );
	    if ( !counting ) {
		waitMsgs.append( new QCString( message ) );
	    }
	}
	if ( counting )
	    steps++;
	waitingForMessages = TRUE;
    } else {
	qDebug( "\tBUG unknown action '%s'!", action.data() );
    }

    if ( !counting ) {
	progressBar->setProgress( progressBar->progress() + 1 );
    }

    return !file->atEnd();
}

void QueuedRequestRunner::desktopMessage( const QCString &message, const QByteArray & )
{
    bool found = FALSE;
    QCString *msg;
    for ( QListIterator<QCString> iter( waitMsgs ); ( msg = iter.current() ) != 0; ++iter ) {
	if ( *msg == message ) {
	    found = TRUE;
	    break;
	}
    }
    if ( found ) {
	waitMsgs.clear();
	waitingForMessages = FALSE;
	QTimer::singleShot( 100, this, SLOT(process()) );
    }
}

