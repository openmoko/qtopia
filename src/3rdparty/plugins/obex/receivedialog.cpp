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
    : QMainWindow( parent, name, FALSE )//, WStyle_StaysOnTop )
{
    setCaption( tr("Infrared receive") );

    w = new ReceiveDialogBase(this, "irReceive");
    setCentralWidget(w);
    
    server = irserver;
    connect( server, SIGNAL(received(const QString&, const QString&) ), 
	     this, SLOT( received() ) );
    connect( server, SIGNAL(receiving(int, const QString&, const QString&) ), 
	     this, SLOT( receiving(int, const QString&, const QString&) ) );

    
    connect( w->cancelButton, SIGNAL( clicked() ), this, SLOT( cancelPressed() ) );
    connect( w->discardButton, SIGNAL( clicked() ), this, SLOT( discardPressed() ) );
    connect( w->saveButton, SIGNAL( clicked() ), this, SLOT( savePressed() ) );
    connect( w->openButton, SIGNAL( clicked() ), this, SLOT( openPressed() ) );
    
    w->applyFrame->hide();
}

ReceiveDialog::~ReceiveDialog()
{
}

void ReceiveDialog::statusMsg(const QString &str)
{
//    setStatus( str );
    if ( !isVisible() )
	Global::statusMessage( str );
}

void ReceiveDialog::receiving( int size, const QString &filename, const QString &mimetype )
{
    connect( server, SIGNAL( statusMsg(const QString &) ), this, SLOT( statusMsg(const QString &) ) );
    connect( server, SIGNAL( progress(int) ), this, SLOT( progress(int) ) );

    // reset widget states
    w->cancelButton->setEnabled(TRUE);
    w->openButton->setEnabled(TRUE);
    w->progressFrame->show();
    w->applyFrame->hide();
    w->detailsLabel->setText("");

    setInfo(size, filename, mimetype);
}

void ReceiveDialog::finished()
{
    disconnect( server, SIGNAL( statusMsg(const QString &) ), this, SLOT( statusMsg(const QString &) ) );
    disconnect( server, SIGNAL( progress(int) ), this, SLOT( progress(int) ) );
}

void ReceiveDialog::setInfo( int s, const QString &fn, const QString &t )
{
    w->filename->setText( fn );
    QString sizeStr = (s != 0 ? QString::number( s ) : tr( "unknown" ) );
    w->size->setText( sizeStr );
    QString typeStr = (t.isNull() ? tr( "unknown" ) : t );
    w->type->setText( typeStr );
    
    if ( s )  {
	w->progressBar->reset();
	w->progressBar->setTotalSteps(s);
    } else {
	w->progressBar->hide();
    }
    totalSize = s;

}

void ReceiveDialog::progress( int s )
{
    QString progressStr;
    if ( totalSize && s != totalSize ) {
	w->progressBar->setProgress( s );
	progressStr = QString("%1/%1").arg(s).arg(totalSize);
    } else {
	progressStr = QString::number(s);
    }
    w->size->setText(progressStr);

    if ( !isVisible() ) {
	int percent;
	if ( totalSize )
	    percent = ( s  * 100 / totalSize );
	else
	    percent = 100;
	
	QString str = w->filename->text();
	if ( str.length() > 20 ) {
	    str.truncate(17);
	    str + "...";
	}
	str += QString(" (%1\%)").arg(percent);
	Global::statusMessage(str);
    }
}

static QString vcalInfo( const QString &filename, bool *todo, bool *cal )
{
    *cal = *todo = FALSE;
    
    QValueList<Event> events = Event::readVCalendar( filename );

    QString desc;
    if ( events.count() ) {
	*cal = TRUE;
	
	desc = ReceiveDialog::tr( "You received an event for:\n" );
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
	
	desc += ReceiveDialog::tr( "You received a todo item:\n" );
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
    progress( totalSize );

    MimeType mt( w->filename->text() );
    QString service = "Receive/"+mt.id();
    QCString receiveChannel = Service::channel(service);
    application = QString::null;
    if ( receiveChannel.isEmpty() ) {
	// Special cases...
	// ##### should split file, or some other full fix
	if ( mt.id() == "text/x-vCalendar" ) {
	    bool calendar, todo;
	    QString desc = vcalInfo( "/tmp/obex/" + w->filename->text(), &todo, &calendar );
	    if ( calendar ) {
		receiveChannel = Service::channel(service+"-Events");
		application = "datebook";
		w->detailsLabel->setText( desc );
	    } else if ( todo ) {
		receiveChannel = Service::channel(service+"-Tasks");
		application = "todolist";
		w->detailsLabel->setText( desc );
	    }
	}
	if ( receiveChannel.isEmpty() ) {
	    QCString openchannel = Service::channel("Open/"+mt.id());
	    if ( openchannel.isEmpty() )
		w->openButton->setEnabled(FALSE);
	}
    } else {
	AppLnk app = Service::appLnk(service);
	if ( app.isValid() )
	    w->detailsLabel->setText( app.name() );
    }
    
    w->progressFrame->hide();
    w->applyFrame->show();

    if ( !receiveChannel.isEmpty() ) {
	// Send immediately
	AppLnk lnk(Service::appLnk(service));
	QCopEnvelope e( receiveChannel, "receiveData(QString,QString)");
	e << QString( "/tmp/obex/" + w->filename->text() ) << mt.id();
	close();
    }
    
    finished();
}

void ReceiveDialog::failed()
{
    w->cancelButton->setEnabled( FALSE );
    w->detailsLabel->setText( tr("Failed") );
    finished();
}

void ReceiveDialog::cancelPressed()
{
    server->cancel();
    failed();
    close();
}

void ReceiveDialog::discardPressed()
{
    close();
}

void ReceiveDialog::savePressed()
{
    save(FALSE);
    close();
}

void ReceiveDialog::openPressed()
{
    save(TRUE);
    close();
}

void ReceiveDialog::save(bool open)
{
    QString fn = w->filename->text();
    
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


