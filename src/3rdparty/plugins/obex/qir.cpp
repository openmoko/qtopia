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
#include "sendwindow.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>

#include <qdir.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include <stdlib.h>

#define BEAM_TRAY_ID ((int) 0x43535345)

//#define QTOPIA_IR_DEBUG

static void notifyBeamBegin()
{
    QCopEnvelope("Qt/Tray", "setIcon(int,QPixmap)")
	<< BEAM_TRAY_ID << Resource::loadPixmap("beam");
    QPEApplication::setTempScreenSaverMode(QPEApplication::DisableSuspend);
}

static void notifyBeamDone()
{
    QCopEnvelope("Qt/Tray", "remove(int)")
	<< BEAM_TRAY_ID;
    QPEApplication::setTempScreenSaverMode(QPEApplication::Enable);
}

QIr::QIr( QObject *parent, const char *name )
    : QObject( parent, name ),
      receiveWindow( 0 ),
      sendWindow( 0 )
{
    obexServer = new QIrServer( this );
    connect( obexServer, SIGNAL( receiveInit() ), this, SLOT( receiveInit() ) );
    connect( obexServer, SIGNAL( received( const QString &, const QString & ) ),
 	     this, SLOT( received( const QString &, const QString & ) ) );
    connect( obexServer, SIGNAL( receiveError() ), this, SLOT( receiveError() ) );

    connect( obexServer, SIGNAL( beamDone() ), this, SLOT( beamDone() ) );
    connect( obexServer, SIGNAL( beamError() ), this, SLOT( beamError() ) );

    obexChannel = new QCopChannel( "QPE/Obex", this );
    connect( obexChannel, SIGNAL(received(const QCString&, const QByteArray&)),
	     this, SLOT(obexMessage(const QCString&, const QByteArray&)) );

    trayChannel = new QCopChannel( "Qt/Tray", this );
    connect( trayChannel, SIGNAL(received(const QCString&, const QByteArray&)),
	     this, SLOT(traySocket(const QCString&, const QByteArray&)) );

    ips = new IrPowerSave(this);
}

QIr::~QIr()
{
    if ( sendWindow )
	delete sendWindow;

    if ( receiveWindow )
	delete receiveWindow;
    
    delete ips;
}

void QIr::obexMessage( const QCString &msg , const QByteArray &data )
{
#ifdef QTOPIA_IR_DEBUG
    qDebug("received message on QPE/Obex: msg=%s", msg.data() );
#endif
    QDataStream stream( data, IO_ReadOnly );
    QString name;
    QString filename;
    QString mimetype;
    if ( msg == "send(QString,QString,QString)" ) {
        stream >> name >> filename >> mimetype;
    } else if ( msg == "send(QString)" ) {
        QString filename;
        stream >> filename;
    } else {
	return;
    }
    
    if ( !filename.isEmpty() ) {
	QFileInfo fi(filename);
#ifdef QTOPIA_IR_DEBUG
	qDebug("sending obex object, filename=%s, mimetype=%s", filename.latin1(), mimetype.latin1());
#endif
	if ( obexServer->state() != QIrServer::Ready ) {
	    QString desc = name;
	    if ( name.isEmpty() )
		desc = filename;
	    
	    QString str = tr("Busy, dropping request %1").arg(desc);
	    Global::statusMessage(str);
	    return;
	}
	
	if ( !sendWindow )
	    sendWindow = new SendWindow(obexServer, 0, "irSend");
	
	// anoher test, just in case we used some time to create the sendwindow (and an incoming request came)
	if ( obexServer->state() == QIrServer::Ready ) {
	    BeamItem item;
	    item.name = name;
	    item.fileName = fi.fileName();
	    item.mimeType = mimetype;
	    item.fileSize = fi.size();
	    
	    sendWindow->add( item );
	
	    ips->initBeam();

	    sendWindow->initBeam();
	    obexServer->beam( filename, mimetype );
	    sendWindow->beamingFirst();
	    notifyBeamBegin();
	} 
    }
}

void QIr::traySocket( const QCString &msg, const QByteArray &data)
{
    QDataStream stream( data, IO_ReadOnly );
    int id = 0;
    QPoint p;
    
    if ( msg == "popup(int,QPoint)" ) {
	stream >> id >> p;
    } else if ( msg == "clicked(int,QPoint)" || msg == "doubleClicked(int,QPoint)" ) {
	stream >> id >> p;
    }

    if ( id == BEAM_TRAY_ID ) {
	
	switch( obexServer->state() ) {
	    case QIrServer::Beaming:	    
		if ( sendWindow->isVisible() )
		    sendWindow->raise();
		else
		    sendWindow->showMaximized();
		break;
	    case QIrServer::Receiving:
		if ( receiveWindow->isVisible() ) {
		    receiveWindow->raise();
		} else {
		    receiveWindow->showMaximized();
		    Global::statusMessage( "" );
		}
		break;
	   default:
		notifyBeamDone();
		break;
	}
    }
}

void QIr::receiveInit()
{
    if ( !receiveWindow )
      receiveWindow = new ReceiveDialog(obexServer, 0, "irReceive");

    notifyBeamBegin();
}

void QIr::received( const QString&, const QString &)
{
    if ( receiveWindow->isVisible() ) {
	receiveWindow->raise();
    } else {
	receiveWindow->showMaximized();
	Global::statusMessage( "" );
    }

    notifyBeamDone();
    ips->beamingDone(TRUE);
}

void QIr::receiveError()
{
    if ( !receiveWindow ) {
	qDebug("!!!bug!!! got receive error before receivewindow was created");
	return;
    }
    
    receiveWindow->failed();
    // let's show the window
    if ( receiveWindow->isVisible() )
	receiveWindow->raise();
    else
	receiveWindow->showMaximized();

    notifyBeamDone();
    ips->beamingDone(TRUE);
}

void QIr::beamDone()
{
    sendWindow->itemBeamed();
    
    if ( sendWindow->count() ) {
	QTimer::singleShot(0, this, SLOT(beamNext()) );
    } else {
	sendWindow->finished();
	ips->beamingDone();
	notifyBeamDone();
    }
}

void QIr::beamNext()
{
    if ( !sendWindow->count() )
	return;

    BeamItem item = sendWindow->next();
    obexServer->beam( item.fileName, item.mimeType );
    sendWindow->beamingFirst();
}

void QIr::beamError()
{
    if ( !sendWindow ) {
	qDebug("!!!bug!!! got beam error before beamwindow was created");
	return;
    }
    
    sendWindow->failed();
    notifyBeamDone();
    ips->beamingDone();
}


/*    IrPowerSave	*/

const char* servicecmd = "/etc/rc.d/init.d/irda";
const char* servicefile = "/var/lock/subsys/irda";

IrPowerSave::IrPowerSave( QObject *parent, const char *name)
    : QObject(parent, name)
{
    running = FALSE;
    inUse = FALSE;
    state = Off;
    time = 0;
    timer = 0;

    if ( QFile::exists(servicefile) ) {
	state = On;
	running = TRUE;
    }
    
    QCopChannel *obexChannel = new QCopChannel( "QPE/Obex", this );
    connect( obexChannel, SIGNAL(received(const QCString&, const QByteArray&)),
	     this, SLOT(obexMessage(const QCString&, const QByteArray&)) );

}

IrPowerSave::~IrPowerSave()
{
    // save state
}

void IrPowerSave::initBeam()
{
    if ( !running )
	service("start"); // No tr

    inUse = TRUE;
}

void IrPowerSave::beamingDone(bool received)
{
    inUse = FALSE;

    switch( state ) {
	case Off:
	    service("stop"); // No tr
	    break;
	case On:
	    break;
	case On1Item:
	    if ( received ) {
		state = Off;
		service("stop"); // No tr
	    }
	    break;
	case OnMinutes:
	    break;
    }
}

void IrPowerSave::obexMessage(const QCString &msg, const QByteArray &data)
{
    State newState = state;;

    if ( msg == "turnOn()") {
	newState = On;
    } else if ( msg == "turnOff()") {
	newState = Off;
    } else if ( msg == "turnOn1Item()" ) {
	newState = On1Item;
    } else if ( msg == "turnOnTimed(int)") {
	newState = OnMinutes;
	QDataStream stream( data, IO_ReadOnly );
	stream >> time;
	time *= (60*1000);
    }
    
    if ( newState != state )
	applyReceiveState( newState );
}

/*  Apply requested beam receive state.  Be careful not to
    mess up any ongoing transmissions	*/
void IrPowerSave::applyReceiveState(State s)
{
    // The user might choose on5, then on1, which is why we need to stop the timer.
    // Other combinations will be handled ok. 
    if ( timer )
	timer->stop();

    state = s;

    switch( state ) {
	case Off:
	    if ( !inUse && running )
		service("stop"); // No tr
	    break;
	case On:
	    if ( !running )
		service("start"); // No tr
	    break;
	case On1Item:
	    if ( !running ) {
		service("start"); // No tr
	    }
	    break;
	case OnMinutes:
	    if ( !timer ) {
		timer = new QTimer(this);
		connect(timer, SIGNAL(timeout() ), this, SLOT(timeout()) );
	    }
	    
	    if ( !running )
		service("start"); // No tr
	    timer->start(time, TRUE);
	    break;
    }
}

void IrPowerSave::timeout()
{
    timer->stop();
    if ( state != Off )
	applyReceiveState( Off );
}

void IrPowerSave::service(const QString& command)
{
    system((QString(servicecmd) + " " + command).latin1());

    if ( command == "start" ) { // No tr
	qDebug("started ir service");
	running = TRUE;
    } else {
	qDebug("stopped ir service");
	running = FALSE;
	// Don't really need to set this one here, but it's safer
	inUse = FALSE;
	QCopEnvelope("QPE/Obex", "turnedOff()");
    }
}

