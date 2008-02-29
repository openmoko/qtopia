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
#include "receivedialog.h"
#include "qirserver.h"

#include <qtopia/applnk.h>
#include <qtopia/mimetype.h>
#include <qtopia/filemanager.h>
#include <qtopia/global.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/private/contact.h>
#include <qtopia/private/event.h>
#include <qtopia/private/task.h>
#include <qtopia/services.h>

#include <qpushbutton.h>
#include <qframe.h>
#include <qlabel.h>
#include <qfile.h>
#include <qprogressbar.h>

ReceiveDialog::ReceiveDialog( QIrServer* irserver, QWidget *parent, const char *name )
    : ReceiveDialogBase( parent, name, FALSE )//, WStyle_StaysOnTop )
{
    server = irserver;
    connect( cancelButton1, SIGNAL( clicked() ), this, SLOT( cancelPressed() ) );
    connect( cancelButton2, SIGNAL( clicked() ), this, SLOT( cancelPressed() ) );
    connect( saveButton, SIGNAL( clicked() ), this, SLOT( savePressed() ) );
    connect( openButton, SIGNAL( clicked() ), this, SLOT( openPressed() ) );
    
    applyFrame->hide();
}

ReceiveDialog::~ReceiveDialog()
{
}
    
void ReceiveDialog::setInfo( int s, const QString &fn, const QString &t )
{
    filename->setText( fn );
    QString sizeStr = (s != 0 ? QString::number( s ) : tr( "unknown" ) );
    size->setText( sizeStr );
    QString typeStr = (t.isNull() ? tr( "unknown" ) : t );
    type->setText( typeStr );
    
    if ( s )  {
	progressBar->setTotalSteps(s);
    } else {
	progressBar->hide();
    }
    totalSize = s;

    connect( server, SIGNAL( progress(int) ), this, SLOT( progress(int) ) );
    connect( server, SIGNAL(received(const QString&, const QString&) ), 
	     this, SLOT( received() ) );

}

void ReceiveDialog::progress( int s )
{
    QString progressStr;
    if ( totalSize && s != totalSize ) {
	progressBar->setProgress( s );
	progressStr = QString("%1/%1").arg(s).arg(totalSize);
    } else {
	progressStr = QString::number(s);
    }
    size->setText(progressStr);
}

static QString vcalInfo( const QString &filename, bool *todo, bool *cal )
{
    *cal = *todo = FALSE;
    
    QValueList<Event> events = Event::readVCalendar( filename );

    QString desc;
    if ( events.count() ) {
	*cal = TRUE;
	
	desc = QObject::tr( "You received an event for:\n" );
	int i = 0;
	for( QValueList<Event>::ConstIterator it = events.begin(); it != events.end() && i < 3; ++it ) {
	    desc += "\n" + (*it).description();
	    i++;
	}
	if ( i == 3 )
	    desc += "\n...";
    }

    QValueList<Task> tasks = Task::readVCalendar( filename );
    
    if ( tasks.count() ) {
	*todo = TRUE;
	if ( *cal )
	    desc += "\n";
	
	desc += QObject::tr( "You received a todo item:\n" );
	int i = 0;
	for( QValueList<Task>::ConstIterator it = tasks.begin(); it != tasks.end() && i < 3; ++it ) {
	    desc += "\n" + (*it).description();
	    i++;
	}
	if ( i == 3 )
	    desc += "\n...";
    }
    
    return desc;    
}

void ReceiveDialog::received()
{
    QCopEnvelope env("QPE/Obex","received()");

    MimeType mt( filename->text() );
    QString service = "Receive/"+mt.id();
    QCString receiveChannel = Service::channel(service);
    if ( receiveChannel.isEmpty() ) {
	// Special cases...
	// ##### should split file, or some other full fix
	if ( mt.id() == "text/x-vCalendar" ) {
	    bool calendar, todo;
	    QString desc = vcalInfo( "/tmp/obex/" + filename->text(), &todo, &calendar );
	    if ( calendar ) {
		receiveChannel = Service::channel(service+"-Events");
		application = "datebook";
		detailsLabel->setText( desc );
	    } else if ( todo ) {
		receiveChannel = Service::channel(service+"-Tasks");
		application = "todolist";
		detailsLabel->setText( desc );
	    }
	}
	if ( receiveChannel.isEmpty() ) {
	    QCString openchannel = Service::channel("Open/"+mt.id());
	    if ( openchannel.isEmpty() )
		openButton->setEnabled(FALSE);
	    progressFrame->hide();
	    applyFrame->show();
	}
    }

    if ( !receiveChannel.isEmpty() ) {
	// Send immediately
	AppLnk lnk(Service::appLnk(service));
	QCopEnvelope e( receiveChannel, "receiveData(QString,QString)");
	e << QString( "/tmp/obex/" + filename->text() ) << mt.id();
	accept();
    }
}

void ReceiveDialog::cancelPressed()
{
    server->cancel();
    reject();
    delete this;
}

void ReceiveDialog::savePressed()
{
    save(FALSE);
    accept();
    delete this;
}

void ReceiveDialog::openPressed()
{
    save(TRUE);
    accept();
    delete this;
}

void ReceiveDialog::save(bool open)
{
    QString fn = filename->text();
    
    // let's check the type. If it's a known one we pass it to the app, otherwise
    // add it to documents
    if ( !application.isEmpty() ) {
	QCString channel = QString("QPE/Application/"+application).data();
	QCopEnvelope e( channel, "setDocument(QString)");
	e << QString( "/tmp/obex/" + fn );
    } else {
	int pos = fn.findRev( "/" );
	if ( pos != -1 )
	    fn = fn.mid( pos );
	QFile f("/tmp/obex/" + fn);
	if ( f.open(IO_ReadOnly) ) {
	    DocLnk doc;
	    doc.setType( MimeType( fn ).id() );
	    // strip off extension
	    pos = fn.findRev( "." );
	    if ( pos != -1 )
		fn = fn.left( pos );
	    doc.setName( fn );
	    FileManager fm;
	    fm.saveFile( doc, f.readAll() );
	    doc.writeLink();
	    if ( open ) {
		QCopEnvelope e( Service::channel("Open/"+doc.type()), "setDocument(QString)");
		e << doc.linkFile();
	    }
	}
    }
}


