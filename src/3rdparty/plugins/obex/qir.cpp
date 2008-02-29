/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qir.h"
#include "qirserver.h"
#include "receivedialog.h"
#include "senddialog.h"

#include <qpe/qpeapplication.h>

#include <qdir.h>
#include <qmessagebox.h>
#include <qcopchannel_qws.h>

QIr::QIr( QObject *parent, const char *name )
    : QObject( parent, name ),
      receiveDialog( 0 ),
      sendDialog( 0 )
{
    obexServer = new QIrServer( this );
//     connect( obexServer, SIGNAL( received( const QString &, const QString & ) ),
// 	     this, SLOT( received( const QString &, const QString & ) ) );
    connect( obexServer, SIGNAL( receiving( int, const QString &, const QString & ) ),
	     this, SLOT( receiving( int, const QString &, const QString & ) ) );
    connect( obexServer, SIGNAL(progress(int) ),
	     this, SLOT( progress(int) ) );
    connect( obexServer, SIGNAL( done() ), this, SLOT( done() ) );

    obexChannel = new QCopChannel( "QPE/Obex", this );
    connect( obexChannel, SIGNAL(received(const QCString&, const QByteArray&)),
	     this, SLOT(obexMessage(const QCString&, const QByteArray&)) );

}

QIr::~QIr()
{
    if ( receiveDialog )
	delete receiveDialog;
}

void QIr::receiving( int size, const QString &filename, const QString &mimetype)
{
  qDebug("receiving obex object, size=%d, filename=%s, mimetype=%s", 
	 size, filename.latin1(), mimetype.latin1());

  if ( receiveDialog )
      delete receiveDialog;
  receiveDialog = new ReceiveDialog( obexServer );
  connect( receiveDialog, SIGNAL( destroyed() ), this, SLOT( dialogDestroyed() ) );

  receiveDialog->setInfo( size, filename, mimetype );
  receiveDialog->showMaximized();
}

void QIr::progress( int size )
{
  qDebug("progress obex object, size=%d", size );

  if ( receiveDialog )
      receiveDialog->progress( size );
}


void QIr::obexMessage( const QCString &msg , const QByteArray &data )
{
    qDebug("received message on QPE/Obex: msg=%s", msg.data() );
    QDataStream stream( data, IO_ReadOnly );
    QString description;
    QString filename;
    QString mimetype;
    if ( msg == "send(QString,QString,QString)" ) {
        stream >> description >> filename >> mimetype;
	qDebug("sending obex object, filename=%s, mimetype=%s", filename.latin1(), mimetype.latin1());
    } else if ( msg == "send(QString)" ) {
        QString filename;
        stream >> filename;
	qDebug("sending obex object, filename=%s", filename.latin1());
	// ### do we need to get to know the mimetype???
	obexServer->beam( filename, QString::null );
    } else {
	qDebug("wrong qcop call");
	return;
    }
    if ( description.isEmpty() ) 
	description = filename;
    if ( !filename.isEmpty() && !sendDialog && !receiveDialog ) {
	obexServer->beam( filename, mimetype );
	sendDialog = new SendDialog( obexServer, description );
	connect( sendDialog, SIGNAL( destroyed() ), this, SLOT( sendDialogDestroyed() ) );
	sendDialog->showMaximized();
    }
}


void QIr::dialogDestroyed()
{
    receiveDialog = 0;
}


void QIr::sendDialogDestroyed()
{
    sendDialog = 0;
}

void QIr::done()
{
    if ( sendDialog )
	delete sendDialog;
    sendDialog = 0;
}
