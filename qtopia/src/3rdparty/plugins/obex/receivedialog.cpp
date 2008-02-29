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
#include <qfile.h>
#include <qtextstream.h>

//#include <qmessagebox.h>

ReceiveDialog::ReceiveDialog( QIrServer* irserver, QWidget *parent, const char *name )
    : QMainWindow( parent, name, FALSE )//, WStyle_StaysOnTop )
{
    setCaption( tr("Infrared receive") );

    w = new ReceiveDialogBase(this, "irReceive");
    setCentralWidget(w);
    
    server = irserver;
    connect( server, SIGNAL(fileComplete()), this, SLOT(fileComplete()) );
    connect( server, SIGNAL( statusMsg(const QString&) ), this, SLOT( statusMsg(const QString&) ) );
    connect( server, SIGNAL( progressReceive(int) ), this, SLOT( progress(int) ) );
    /*
    connect( server, SIGNAL(received(const QString&,const QString&) ), 
	     this, SLOT( received() ) );
	     */
    connect( server, SIGNAL(receiving(int,const QString&,const QString&) ), 
	     this, SLOT( receiving(int,const QString&,const QString&) ) );
    connect( server, SIGNAL(receiveDone()), this, SLOT(done()) );

    
    connect( w->cancelButton, SIGNAL( clicked() ), this, SLOT( cancelPressed() ) );
    connect( w->discardButton, SIGNAL( clicked() ), this, SLOT( discardPressed() ) );
    connect( w->saveButton, SIGNAL( clicked() ), this, SLOT( savePressed() ) );
    connect( w->openButton, SIGNAL( clicked() ), this, SLOT( openPressed() ) );
    
    reset();
}

ReceiveDialog::~ReceiveDialog()
{
}

void ReceiveDialog::fileComplete()
{
    progress( totalSize );
}

void ReceiveDialog::statusMsg(const QString &str)
{
//    setStatus( str );
    if ( !isVisible() )
	Global::statusMessage( str );
}

void ReceiveDialog::receiving( int size, const QString &filename, const QString &mimetype )
{
    /*
    connect( server, SIGNAL( statusMsg(const QString&) ), this, SLOT( statusMsg(const QString&) ) );
    connect( server, SIGNAL( progressReceive(int) ), this, SLOT( progress(int) ) );

    */
    //reset();

    //  Display information about the incoming file.
    setInfo(size, filename, mimetype);
    if( !receivedFiles.contains( filename ) )
	receivedFiles += filename;
}

void ReceiveDialog::reset()
{
    // reset widget states
    w->cancelButton->setEnabled(TRUE);
    w->openButton->setEnabled(TRUE);
    w->saveButton->setEnabled(TRUE);
    w->detailsLabel->setText("");
    if( isVisible() ) {
	w->progressFrame->show();
	w->applyFrame->hide();
    }
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
    if ( !isVisible() ) {
	
	QString str = w->filename->text();
	if ( str.length() > 20 ) {
	    str.truncate(17);
	    str + "...";
	}
	if( totalSize > 0 && s <= totalSize ) {
	    int percent = ( s  * 100 / totalSize );
	    str += QString(" (%1\%)").arg(percent);
	}
	Global::statusMessage(str);
    } else {
	QString progressStr;
	if ( totalSize && s != totalSize ) {
	    w->progressBar->setProgress( s );
	    progressStr = QString("%1/%1").arg(s).arg(totalSize);
	} else {
	    progressStr = QString::number(s);
	}
	w->size->setText(progressStr);
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

void ReceiveDialog::done()
{
    //finished receiving.

    // combine multiple vcards into a single file
    receivedFiles = catFilesForType( receivedFiles, "text/x-vCard", "vcardset.vcf" );
    receivedFiles = catFilesForType( receivedFiles, "text/x-vCalendar", "vcalset.vcs" );

    w->progressFrame->hide();
    w->applyFrame->show();

    for( QStringList::ConstIterator it = receivedFiles.begin() ; it != receivedFiles.end() ; ++it ) {
	MimeType mt( QIrServer::landingZone() + "/" + *it );
	QString service = "Receive/"+mt.id();
	QCString receiveChannel = Service::channel(service);
	application = QString::null;
	if ( receiveChannel.isEmpty() ) {
	    // Special cases...
	    // ##### should split file, or some other full fix
	    if ( mt.id() == "text/x-vCalendar" ) {
		bool calendar, todo;
		QString desc = vcalInfo( QIrServer::landingZone() + "/" + *it, &todo, &calendar );
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

	if ( !receiveChannel.isEmpty() ) {
	    // Send immediately
	    AppLnk lnk(Service::appLnk(service));
	    QCopEnvelope e( receiveChannel, "receiveData(QString,QString)");
	    e << QString( QIrServer::landingZone() + "/" + *it ) << mt.id();
	    close();
	    reset();
	}
    }
    receivedFiles.clear();
}

//concatenate inputFiles of a certain mimeType into a single outFile. returns the new list of filenames
QStringList ReceiveDialog::catFilesForType( const QStringList &inputFiles, const QString &mimeType, const QString &outFile )
{
    QStringList newFiles = inputFiles;
    QStringList typeFiles;
    for( QStringList::ConstIterator it = inputFiles.begin() ; it != inputFiles.end() ; ++it ) {
	MimeType mt( *it );
	const QString mtl = mimeType.lower();
	if( mt.id().lower() == mtl )
	    typeFiles += *it;
    }
    if( typeFiles.count() <= 1 )
	return inputFiles; //only one file matching mimeType, no need to concat

    QFile of( QIrServer::landingZone() + "/" + outFile );
    if( !of.open( IO_WriteOnly ) )
	return inputFiles;
    QTextStream ostream( &of );

    for( QStringList::ConstIterator it = typeFiles.begin(); it != typeFiles.end() ; ++it ) {
	QFile inf( QIrServer::landingZone() + "/" + *it );
	if( inf.open( IO_ReadOnly ) ) {
	    QTextStream istream( &inf );
	    QString c = istream.read();
	    if( !c.isNull() )
		ostream << c;
	    inf.close();
	}
	inf.remove();
	newFiles.remove( *it );
    }
    of.close();
    newFiles += outFile;
    return newFiles;
}

void ReceiveDialog::failed()
{
    if( isVisible() ) {
	w->detailsLabel->setText( tr("Receive Failed") );
	w->progressFrame->hide();
	w->cancelButton->setEnabled( FALSE );
	w->saveButton->setEnabled(FALSE);
	w->openButton->setEnabled(FALSE);
	w->applyFrame->show();
    } else {
	Global::statusMessage( tr("Receive Failed") );
    }
}

void ReceiveDialog::cancelPressed()
{
    server->cancelReceive();
    failed();
    close();
    reset();
    QString fn = QIrServer::landingZone() + "/" + w->filename->text();
    QFile( fn ).remove();
}

void ReceiveDialog::discardPressed()
{
    close();
    reset();
    QString fn = QIrServer::landingZone() + "/" + w->filename->text();
    QFile( fn ).remove();
}

void ReceiveDialog::savePressed()
{
    save(FALSE);
    close();
    reset();
}

void ReceiveDialog::openPressed()
{
    save(TRUE);
    close();
    reset();
}

void ReceiveDialog::save(bool open)
{
    QString fn = w->filename->text();
    
    // let's check the type. If it's a known one we pass it to the app, otherwise
    // add it to documents
    if ( !application.isEmpty() ) {
	QCString channel = QString("QPE/Application/"+application).data();
	QCopEnvelope e( channel, "setDocument(QString)");
	e << QString( QIrServer::landingZone() + "/" + fn );
    } else {
	int pos = fn.findRev( "/" );
	if ( pos != -1 )
	    fn = fn.mid( pos );
	QFile f( QIrServer::landingZone() + "/" + fn);
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
