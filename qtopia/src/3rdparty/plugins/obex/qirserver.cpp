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
#include "qirserver.h"

#include <qtopia/mimetype.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/private/task.h>
#include <qtopia/storage.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/services.h>

#include <qsocketnotifier.h>
#include <qfile.h>
#include <qtimer.h>
#include <qmessagebox.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <qtopia/global.h>

#define QTOPIA_DEBUG_OBEX

extern "C" { 
#include "openobex/obex.h"
}

class QObexBase :public QObject
{
    Q_OBJECT;
public:
    QObexBase( QObject *parent = 0, const char *name = 0 );
    ~QObexBase();

signals:
    void error();
    void statusMsg(const QString &);
    void aboutToDelete();

public slots:
    void abort();

protected slots:
    void deleteMeLater();
    void processInput();
    
protected:
    virtual void doPending() = 0;
    void connectSocket();

    obex_t *self;

    bool finished;
    bool aborted;

private slots:
    void deleteThis();
};



QObexBase::QObexBase( QObject *parent , const char *name )
    :QObject( parent, name )
{
    finished = FALSE;
    aborted = FALSE;
}

QObexBase::~QObexBase()
{
    OBEX_Cleanup( self );
}

void QObexBase::connectSocket()
{
    QSocketNotifier *sn = new QSocketNotifier( OBEX_GetFD( self ),
					       QSocketNotifier::Read,
					       this );
    connect( sn, SIGNAL(activated(int)), this, SLOT(processInput()) );
}


void QObexBase::deleteMeLater()
{
    QTimer::singleShot( 0, this, SLOT(deleteThis()) );
}


void QObexBase::deleteThis()
{
    emit aboutToDelete();
    delete this;
}

void QObexBase::abort()
{
    aborted = TRUE;
    finished = TRUE;
}


void QObexBase::processInput()
{
    if ( !aborted )
	OBEX_HandleInput( self, 0 );

    if ( aborted )
	deleteMeLater();
    if ( finished )
	doPending();
}

static void qobex_sender_callback(obex_t *handle, obex_object_t *obj, int mode, int event, int obex_cmd, int obex_rsp);

class QObexSender : public QObexBase
{
    Q_OBJECT;
public:
    QObexSender( QObject *parent = 0, const char *name = 0 );
    ~QObexSender();

    void beam( const QString& filename, const QString& mimetype );

signals:
    void done();
    void progress(int);

private slots:
    void tryConnecting(); 

private:
    friend void qobex_sender_callback(obex_t *handle, obex_object_t *obj, int mode, int event, int obex_cmd, int obex_rsp);

    enum State { Init, Connecting, Streaming,   
		 Disconnecting, Error };

    void process( obex_object_t *object ) {

	if( object && OBEX_Request(self, object) < 0) {
	    /* Error */
	    qWarning( "could not request" );
	    state = Error;
	    finished = TRUE;
        } else {
	  finished = FALSE;
	}
    }

    void feedStream( obex_object_t *);
    void updateProgress( obex_object_t *);

    void doPending();

    void putFile( const QString&, const QString& );

    QString file_to_send;
    QString mime_to_send;
    QFile *file_being_sent;

    State state;

    int connectCount;
};

class QObexReceiver;

static void qobex_server_callback(obex_t *handle, obex_object_t *object, int mode, int event, int obex_cmd, int obex_rsp);

class QObexServer : public QObexBase
{
  friend void qobex_server_callback(obex_t *handle, obex_object_t *object, int mode, int event, int obex_cmd, int obex_rs);
  Q_OBJECT;
public:
  QObexServer( QObject *parent = 0, const char *name = 0 );
  ~QObexServer();

protected:
    void doPending();


private slots:
    void slotReceiving(bool);	
    void sessionEnded();
    void finishedReceive();

signals:
    void receiving(bool);

private:
    QTimer *m_timer;
    QObexReceiver *m_receiver;
    bool m_receiving;

    void spawnReceiver( obex_t *handle );

    enum State { Init, Connecting, Receiving,   
		 Disconnecting, Finished, Error, Aborted };

    State state;
};





static void qobex_receiver_callback(obex_t *handle, obex_object_t *object, int mode, int event, int obex_cmd, int obex_rsp);

class QObexReceiver : public QObexBase
{
    Q_OBJECT;
public:
    QObexReceiver( obex_t *handle, QObject *parent = 0, const char *name = 0 );
    ~QObexReceiver();

    //a link error finish is a successful finish of a beamed file
    //but terminates with a LINKERR message. some phones do this
    //and create an obex session for each file they want to send
   //sending a link error after each 
    bool linkErrFinish() const { return m_linkErrFinish; }
protected:
    void doPending();

signals:
    void receiving(bool);
    void receiving( int size, const QString & name, const QString & mime );
    void progress( int size );
    void fileComplete();
    //void received( const QString & name, const QString & mime );
    //there is no easy way to emit this signal inbetween sends without
    //screwing devices like the S45 which are tempremental over timing
    //don't need it anyway

private:
    friend void qobex_receiver_callback(obex_t *handle, obex_object_t *object, int mode, int event, int obex_cmd, int obex_rsp);

    enum State { Init, Connecting, Receiving,   
		 Disconnecting, Finished, Error, Aborted };

    void readData( obex_object_t *);
    void updateProgress( obex_object_t *);
    void getHeaders( obex_object_t *object );

    State state;

    QString filename;
    QString mimetype;
    int reclen;
    bool m_linkErrFinish;

    QFile outfile;
};









/*

  QObexSender functions


 */












void QObexSender::feedStream( obex_object_t *object )
{
    static char buf[4096];

    ASSERT( state == Streaming &&  file_being_sent );

    obex_headerdata_t hd;


    int len = file_being_sent->readBlock( buf, 4096 );

    if ( len > 0 ) {
	hd.bs = (uchar*)buf;
	OBEX_ObjectAddHeader(self, object, OBEX_HDR_BODY,
			     hd, len, OBEX_FL_STREAM_DATA);
    } else {
	/* EOF */
	hd.bs = 0;
	OBEX_ObjectAddHeader(self, object, OBEX_HDR_BODY,
			     hd, 0, OBEX_FL_STREAM_DATAEND);


	file_being_sent->close();
	delete file_being_sent;
	file_being_sent = 0;
    }
 }

void QObexSender::updateProgress( obex_object_t * /* obj */)
{
    emit progress( file_being_sent->at() );
}

static bool hasTasks( const QString &filename )
{
    QValueList<Task> tasks = Task::readVCalendar( filename );
    return tasks.count();
}

void QObexSender::putFile( const QString &filename, const QString& mimetype )
{

    obex_headerdata_t hd;
        
    obex_object_t *object = OBEX_ObjectNew(self, OBEX_CMD_PUT);
    if (object == NULL) {
	/* Error */
	qWarning( "null object" );
	state = Error;
	finished = TRUE;
	return;
    }

    file_being_sent = new QFile( filename );
    if ( !file_being_sent->open(IO_ReadOnly) )
    {
        QString msg = tr( "File not found" );
        emit statusMsg( msg );
        emit error();
        return;
    }

    int body_size = file_being_sent->size();

    /* Add length header */
    hd.bq4 = body_size;
    OBEX_ObjectAddHeader(self, object,
            OBEX_HDR_LENGTH, hd, 4, 0);

    /* Add type header */
    hd.bs = (uchar*)mimetype.latin1();
    OBEX_ObjectAddHeader(self, object,
            OBEX_HDR_TYPE, hd, mimetype.length()+1, 0);

    /* Add PalmOS-style application id header */
    // ####### if more PalmOS applications have this problem,
    // ####### a more general solution is required.
    if ( mimetype == "text/x-vCalendar" ) {
        if ( hasTasks( filename ) )
            hd.bq4 = 0x746F646F; // "todo"
        else
            hd.bq4 = 0x64617465; // "date"
        OBEX_ObjectAddHeader(self, object, 0xcf, hd, 4, 0);
    }

    /* Add unicode name header*/

    QString uc = filename + QChar( 0x0 );
    uc = uc.mid( uc.findRev("/")+1 );
    int name_size = uc.length() * 2;

    hd.bs = (uchar*) uc.unicode();
    OBEX_ObjectAddHeader(self, object,
            OBEX_HDR_NAME, hd, name_size, 0);

    state = Streaming;

    hd.bs = 0;
    OBEX_ObjectAddHeader(self, object, OBEX_HDR_BODY,
            hd, 0, OBEX_FL_STREAM_START);
    process( object );
}


static bool transmitDone = FALSE;

void QObexSender::doPending()
{
#ifdef QTOPIA_DEBUG_OBEX
    qDebug( "QObexSender::doPending %d", state );
#endif

    if ( state == Connecting ) {
#ifdef QTOPIA_DEBUG_OBEX
	qDebug( "state Connecting" );
#endif
	// finished connecting; time to send

	putFile( file_to_send, mime_to_send );
	file_to_send = QString::null;

    } else if ( state == Streaming ) {
#ifdef QTOPIA_DEBUG_OBEX
	qDebug( "state Streaming" );
#endif
	//finished streaming

	obex_object_t *object = OBEX_ObjectNew(self, OBEX_CMD_DISCONNECT);
	process( object );
	state = Disconnecting;
    } else if ( state == Disconnecting ) {
#ifdef QTOPIA_DEBUG_OBEX
	qDebug( "state Disconnecting" );
#endif

	OBEX_TransportDisconnect(self);
	state = Init;
	emit done();
	deleteMeLater();

    }
}


static void qobex_sender_callback(obex_t *handle, obex_object_t *obj, int mode, int event, int obex_cmd, int obex_rsp)
{
    QObexSender *sender = (QObexSender*)OBEX_GetUserData( handle );

#ifdef QTOPIA_DEBUG_OBEX
    qDebug( "qobex_sender_callback %p, %p, %p, %d, event %x, cmd %x, rsp %x",
       sender, handle, obj, mode, event, obex_cmd, obex_rsp );
#else
    Q_UNUSED(mode);
    Q_UNUSED(obex_rsp);
#endif

    switch (event) {
    case OBEX_EV_REQDONE:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_REQDONE");
#endif
	sender->finished = TRUE;
	if ( obex_cmd == OBEX_CMD_DISCONNECT ) {
	    transmitDone = TRUE;
	}
	// else if OBEX_CMD_CONNECT, we could give feedback
	break;

    case OBEX_EV_LINKERR:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_LINKRER");
#endif
	// sometime we get a link error after we believed the connection was done.  Ignore this
	// as emitting an error after done does not make sense
	if ( !transmitDone ) {
	    emit sender->error();
	    sender->abort();
	    sender->state = QObexSender::Error;
	} else {
	    sender->abort();
#ifdef QTOPIA_DEBUG_OBEX
	    qDebug("QIrServer:: got link error after done signal, no external signals sent");
#endif
	}
	
	break;

    case OBEX_EV_PROGRESS:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_PROGRESS");
#endif
	// report progress?
	sender->updateProgress( obj );
	break;

	
    case OBEX_EV_STREAMEMPTY:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_STREAMEMPTY");
#endif
	// when streaming: add more
	sender->feedStream( obj );
	break;
	
    default:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("qobex_sender_callback:: did not recongnize event signal %d", event);
#endif
	break;
    }
}


QObexSender::QObexSender( QObject *parent, const char *name )
  :QObexBase( parent, name )
{

  self = OBEX_Init( OBEX_TRANS_IRDA, qobex_sender_callback, OBEX_FL_KEEPSERVER );
  finished = FALSE;
  state = QObexSender::Init;
  OBEX_SetUserData( self, this );
}

QObexSender::~QObexSender()
{
}





void QObexSender::beam( const QString& filename, const QString& mimetype )
{
    file_to_send = filename;
    mime_to_send = mimetype;
    transmitDone = FALSE;

    connectCount = 0;
    tryConnecting();
}



void QObexSender::tryConnecting()
{
    connectCount++;

    if ( aborted ) { deleteMeLater();
	return;
    }
	
    int retc = IrOBEX_TransportConnect(self, "OBEX");

    if ( retc < 0 ) {
	const int normalTry = 5;
	const int maxTry = 20;
	if ( connectCount > maxTry ) {
	    abort();
	    emit error();
	    deleteMeLater();
	} else {
	    QString str;
	    if ( connectCount <= normalTry )
		str = tr("Searching...");
	    else
		str = tr("Beam failed (%1/%2). Retrying...","eg. 1/3").arg(connectCount).arg(maxTry);
	    emit statusMsg( str );
	    // Semi-random retry time to avoid re-collision
	    QTimer::singleShot(200 + rand()%400, this, SLOT(tryConnecting()) );
	}
	return;
    }
#ifdef QTOPIA_DEBUG_OBEX
    qDebug("QObexSender::tryConnecting() - Success");
#endif

    QString str = tr("Sending...");
    emit statusMsg( str );

    connectSocket();

    obex_object_t *object;

    // connecting
    object = OBEX_ObjectNew(self, OBEX_CMD_CONNECT);


    state = QObexSender::Connecting;

    process( object );
}

//-----------------------------------------------------------------------

/*

  QObexServer /  functions

*/

QObexServer::QObexServer( QObject *parent, const char *name )
  :QObexBase( parent, name )
{

    state = Error;
    self = OBEX_Init( OBEX_TRANS_IRDA, qobex_server_callback, 
		      OBEX_FL_KEEPSERVER );
    state = Init;
    OBEX_SetUserData( self, this );

    // XXX To support OBEX other than IrDA, register those services here.
    // XXX (somehow)

    if(IrOBEX_ServerRegister(self, "OBEX") < 0) {
	qWarning( "could not register server" );
	state = Error;
    } else {
      connectSocket();
    }
    m_receiver = 0;
    m_receiving = FALSE;
    m_timer = 0;
}



QObexServer::~QObexServer()
{
    //cleanup
}

void QObexServer::doPending()
{
    //nothing, it's stateless
}

void QObexServer::spawnReceiver( obex_t *handle )
{
    ASSERT( handle == self );

    // we emit this here, as doing in in QObexReceiver could cause race conditions
    
#ifdef QTOPIA_DEBUG_OBEX
    qDebug("QObexServer::spawnReceiver()");
#endif
    if( m_timer && m_timer->isActive() )
	m_timer->stop(); // spawning a new receiver, don't timeout

    m_receiver = new QObexReceiver( handle, this );

    connect( m_receiver, SIGNAL(fileComplete()), parent(), SIGNAL(fileComplete()) );
    connect( m_receiver, SIGNAL(aboutToDelete()), this, SLOT(sessionEnded()) );
    connect( m_receiver, SIGNAL(receiving(int,const QString&,const QString&)),
	     parent(), SIGNAL(receiving(int,const QString&,const QString&)) );
    connect( m_receiver, SIGNAL(progress(int)),
	     parent(), SIGNAL(progressReceive(int)) );
    /* don't enable unless you know what you're doing
    connect( m_receiver, SIGNAL(received(const QString&,const QString&)),
	     parent(), SIGNAL(received(const QString&,const QString&)) );
     */
    
    connect( m_receiver, SIGNAL(receiving(bool)), this, SLOT(slotReceiving(bool)) );
    slotReceiving( TRUE );

    connect( parent(), SIGNAL(abortReceive()), m_receiver, SLOT(abort()) );
    connect( m_receiver, SIGNAL(error()), parent(), SLOT(rError()) );
    
}

void QObexServer::sessionEnded()
{
#ifdef QTOPIA_DEBUG_OBEX
    qDebug("Receive Session Ended");
#endif
    if( m_receiver->linkErrFinish() ) { // session finished with a linkerr
	//work around for Siemens S45
	//if another receive session is not started within 5 seconds,
	//assume there are no more files because there is no definite
	//way to tell
	if( !m_timer ) {
	    m_timer = new QTimer( this );
	    connect( m_timer, SIGNAL(timeout()), this, SLOT(finishedReceive()) );
	}
	m_timer->start( 3000 );
    }
}

void QObexServer::finishedReceive()
{
#ifdef QTOPIA_DEBUG_OBEX
    qDebug("finishedReceive");
#endif
    if( m_timer )
	m_timer->stop();
    m_receiving = FALSE;
    emit receiving( FALSE );
}

void QObexServer::slotReceiving( bool r )
{
#ifdef QTOPIA_DEBUG_OBEX
    qDebug("QObexServer::slotReceiving( %s )", r ?"TRUE" : "FALSE");
#endif
    if( !r && m_receiving && m_receiver->linkErrFinish() ) {
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("LINK ERROR FINISH");
#endif
	 //finished a file successfully due to link error, may not be totally finished
	r = TRUE;
    }
    if( m_receiving != r ) {
	m_receiving = r;
	emit receiving( r );
    }
}

static void qobex_server_callback(obex_t *handle, obex_object_t * object, int mode, int event, int obex_cmd, int obex_rsp)
{

#ifdef QTOPIA_DEBUG_OBEX
    qDebug( "qobex_server_callback %p event %x cmd %x rsp %x", object, event, obex_cmd, obex_rsp );
#else
    Q_UNUSED(object);
    Q_UNUSED(obex_cmd);
    Q_UNUSED(obex_rsp);
#endif
    Q_UNUSED(mode);

    QObexServer* server = 
	(QObexServer*)OBEX_GetUserData( handle );

    switch (event)        {
    case OBEX_EV_ACCEPTHINT:
	{
#ifdef QTOPIA_DEBUG_OBEX
	    qDebug( "#####------ OBEX_EV_ACCEPTHINT ------########" );
#endif

	    server->spawnReceiver( handle );
	}
	break;
    default:
	qWarning( " Unexpected event in server" );
	break;
    }

}



/*

  QObexReceiver functions

*/



QObexReceiver::QObexReceiver( obex_t *handle, QObject *parent, const char *name )
  :QObexBase( parent, name )
{
    state = Error; reclen = 0;
    m_linkErrFinish = FALSE;

    state = Receiving;
    self = OBEX_ServerAccept( handle, qobex_receiver_callback,
			      this );
    connectSocket();

}


QObexReceiver::~QObexReceiver()
{
}



static QString getString( const uchar* bs, int len )
{
    // qDebug( "getString" );
    /*
    for ( int i = 0; i < len; i++ )
	qDebug( "byte %2d : %02x %c", i, bs[i], bs[i] );
    */
    QString s;
    s.setUnicode( (const QChar*)bs, len/2-1 );
    return s;
}

#define HDR_DESCRIPTION 0x05

void QObexReceiver::getHeaders( obex_object_t *object )
{
    uchar hi;
    obex_headerdata_t hv;
    unsigned int hv_size;
    while(OBEX_ObjectGetNextHeader(self, object, &hi, &hv, &hv_size))	{

	switch ( hi ) {

	case OBEX_HDR_LENGTH:
#ifdef QTOPIA_DEBUG_OBEX
	    qDebug( "******** Got length %d", hv.bq4 );
#endif
	    reclen = hv.bq4;
	    break;

	case OBEX_HDR_NAME:
	    filename = getString(hv.bs, hv_size);
#ifdef QTOPIA_DEBUG_OBEX
	    qDebug( "******** Got name %s (%d)", 
			getString(hv.bs, hv_size).latin1(), hv_size );
#endif
	    break;


	case HDR_DESCRIPTION:
#ifdef QTOPIA_DEBUG_OBEX
	    qDebug( "******** Got description %s (%d)", 
			getString(hv.bs, hv_size).latin1(), hv_size );
#endif
	    break;



	case OBEX_HDR_TYPE:
	    mimetype = QString::fromLatin1( (const char*)hv.bs, hv_size );
#ifdef QTOPIA_DEBUG_OBEX
	    qDebug( "******** Got type \"%s\"", mimetype.latin1() );
#endif
	    break;


	default:
#ifdef QTOPIA_DEBUG_OBEX
	    if ( hi < 0x40 ) //unicode header
		qDebug( "******** Got header 0x%x %s (%d)", hi, 
			getString(hv.bs, hv_size).latin1(), hv_size );
	    else if ( (hi & 0xc0) == 0xc0 ) // quantity [eg. cf = PalmOS Creator ID]
		qDebug( "******** Got header 0x%x (%d)", hi, hv.bq4 );
	    else
		qDebug( "******** Got header 0x%x (size=%d)", hi, hv_size );
#endif
	    ;
	}
    }
}

static const char* sizeUnits[] = { 
    QT_TRANSLATE_NOOP( "QObexReceiver", "bytes" ),
    QT_TRANSLATE_NOOP( "QObexReceiver", "KB" ),
    QT_TRANSLATE_NOOP( "QObexReceiver", "MB" ),
    QT_TRANSLATE_NOOP( "QObexReceiver", "GB" ),
    QT_TRANSLATE_NOOP( "QObexReceiver", "TB" )
};

void QObexReceiver::readData( obex_object_t *object )
{
    const uchar* buf;
    int len = OBEX_ObjectReadStream( self, object, &buf );

    if ( filename.isEmpty() ) {
	getHeaders( object );

        //  Now I know the size of the incoming data, make sure there is
        //  sufficient room to store it all. There has to be sufficient 
        //  room in /tmp to receive the file at all, and sufficient room in
        //  the documents directory to store it. 
        
        StorageInfo storageInfo;
        const FileSystem *fileSystem;
        long blocksRequired;
        bool insufficientTemp;
        bool insufficientDocs;
        int response;
        float scaledSize;
        QString scaledUnit;
        int scaling = 0;
                
        fileSystem = storageInfo.fileSystemOf( QIrServer::landingZone() );
        if( fileSystem )
        {
            blocksRequired = 1 + ( reclen / fileSystem->blockSize() );
            insufficientTemp = ( blocksRequired > fileSystem->availBlocks() );
            
            fileSystem = storageInfo.fileSystemOf( QPEApplication::documentDir() );
            blocksRequired = 1 + ( reclen / fileSystem->blockSize() );
            insufficientDocs = ( blocksRequired > fileSystem->availBlocks() );
            
            if( insufficientDocs || insufficientTemp )
            {
                //  Not enough room to receive incoming file!
                
                scaledSize = reclen;
                while( scaling < 4 && scaledSize >= 1024.0 )
                {
                    scaledSize /= 1024.0;
                    scaling++;
                }
                if (qApp)
                    scaledUnit = qApp->translate("QObexReceiver", sizeUnits[ scaling ]);
                else
                    scaledUnit = sizeUnits[ scaling ];
                            
                response = QMessageBox::warning( 0, tr( "Insufficient storage space" ), 
                    tr( "<qt>There is not enough space to store an incoming file "
                        "(%1 %2 required). Would you like to cancel and run the "
                        "cleanup wizard?</qt>", "%1=size of file %2=unit(e.g. kB or MB)" )
                    .arg( scaledSize ).arg( scaledUnit ), QMessageBox::Yes, 
                    QMessageBox::No, (Global::mousePreferred() ? QMessageBox::Cancel : 0));
    
                switch( response )
                {
                    case QMessageBox::Yes:
                    {
                        //  Run the cleanup wizard
                        abort();
                        ServiceRequest req("CleanupWizard", "showCleanupWizard()");
                        req.send();
                        return;
                    }
                    
                    case QMessageBox::No:
                        //  Do nothing.
                        break;
                        
                    case QMessageBox::Cancel:
                        //  Cancel the incoming file.
                        abort();
                        return;
                }
            }
        }
        
        int slash = filename.find( '/', -1 );
	if ( slash > 0 ) {
	    filename = filename.mid( slash+1 );
	}

	if ( mimetype.isNull() ) {
	    QString ext = filename.mid( filename.findRev( '.', -1 )+1 ).lower();
	    if( ext == "vcs" ) {
		mimetype = "text/x-vCalendar";
	    } else if( ext == "vcf" ) {
		mimetype = "text/x-vCard";
	    } else {
		MimeType mt(filename);
		mimetype = mt.id();
	    }
	} 

	if ( filename.isEmpty() )
	    filename = "unnamed"; // No tr

	for( int i = 0 ; i < (int)filename.length() ; ++i)
	    if( filename[i] != '.' && 
			    (filename[i].isPunct() || filename[i].isSpace()) )
		filename[i] = '_'; //ensure it's something valid

#ifdef QTOPIA_DEBUG_OBEX
	qDebug("emit receiving( %d, %s, %s )", reclen, filename.latin1(), mimetype.latin1());
#endif
	emit receiving( reclen, filename, mimetype );

	filename = QIrServer::landingZone() + "/" + filename;
	
	outfile.setName( filename );
	if( !outfile.open( IO_WriteOnly ) )
	    qWarning("Unable to open file %s for IR receive", filename.latin1());
    } 
    if ( len > 0 ) {
	if( outfile.isOpen() ) // if unable to open outfile don't write to it
	    outfile.writeBlock( (const char*)buf, len );
    } else if ( len == 0 ) {
	outfile.close();
	filename = QString::null;
	emit fileComplete();
    } else {
	qWarning( "ERROR reading stream" );
	emit error();
	filename = QString::null;
	outfile.close();
    }
    finished = ( len <= 0 );
}

void QIrServer::clean()
{
#ifdef QTOPIA_DEBUG_OBEX
    qDebug("QIrServer::clean");
#endif
    //just blow it all away and recreate the dir
    QString rmCmd = QString( "rm -rf %1" ).arg( QIrServer::landingZone() );
    QString mkdirCmd = QString( "mkdir -p -m 0755 %1" ).arg( QIrServer::landingZone() );
    system( rmCmd );
    system( mkdirCmd );
}

void QObexReceiver::updateProgress( obex_object_t * /* obj */)
{
    emit progress( outfile.size() );
}


static void qobex_receiver_callback(obex_t *handle, obex_object_t *object, int mode, int event, int obex_cmd, int obex_rsp )
{
#ifdef QTOPIA_DEBUG_OBEX
    qDebug( "qobex_receiver_callback %p event %x cmd %x rsp %x", object, event, obex_cmd, obex_rsp );
#else
    Q_UNUSED(obex_rsp);
#endif
    Q_UNUSED(mode);

    QObexReceiver* receiver = 
	(QObexReceiver*)OBEX_GetUserData( handle );

    switch (event)        {

    case OBEX_EV_STREAMAVAIL:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_STREAMAVAIL");
#endif
	receiver->readData( object );
	break;
    case OBEX_EV_PROGRESS:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_PROGRESS");
#endif
	receiver->updateProgress( object );
	// report progress
	break;

    case OBEX_EV_REQ:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_REQ");
#endif
	/* An incoming request */
	switch(obex_cmd) {
	case OBEX_CMD_CONNECT:
	case OBEX_CMD_DISCONNECT:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("\tOBEX_CMD_CONNECT|OBEX_CMD_DISCONNECT");
#endif
	    /* Dont need to do anything here.
	       Response is already set to
	       success by OBEX_EV_REQHINT event */
	    break;
	case OBEX_CMD_PUT:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("\tOBEX_CMD_PUT");
#endif
	    //We do it all in STREAMAVAIL 
	    //receiver->handlePut(object);
	    break;
	}                 
	break;

    case OBEX_EV_ACCEPTHINT:

#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_ACCEPTHINT");
#endif
	//qFatal( " OBEX_EV_ACCEPTHINT in receiver" );
	OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_IMPLEMENTED,
			  OBEX_RSP_NOT_IMPLEMENTED);
	break;

    case OBEX_EV_REQHINT:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_REQHINT");
#endif

	/* A new request is coming in */
	switch(obex_cmd) {
	    /* Accept some commands! */
	case OBEX_CMD_PUT:
#ifdef QTOPIA_DEBUG_OBJEX
	    qDebug("\tOBEX_CMD_PUT");
#endif
	    OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
	//turn on streaming...
	    OBEX_ObjectReadStream(handle, object, NULL);
	    break;


	case OBEX_CMD_CONNECT:
	case OBEX_CMD_DISCONNECT:
#ifdef QTOPIA_DEBUG_OBJEX
	    qDebug("\tOBEX_CMD_CONNECT|OBEX_CMD_DISCONNECT");
#endif
	    OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
	    break;
                
	default:
#ifdef QTOPIA_DEBUG_OBJEX
	    qDebug("OBEX_CMD_UNKNOWN");
#endif
	    /* Reject any other commands */                       
	    OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_IMPLEMENTED,
			      OBEX_RSP_NOT_IMPLEMENTED);
	    break;
                
	}
	break;

    case OBEX_EV_REQDONE:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_REQDONE");
#endif
	if(obex_cmd == OBEX_CMD_DISCONNECT) {

	    receiver->state = QObexReceiver::Finished;
	    receiver->finished = TRUE;
	    // Disconnect transport:
	    OBEX_TransportDisconnect( handle );
	}
	break;

    case OBEX_EV_LINKERR:
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("OBEX_EV_LINKERR");
#endif
	if ( !receiver->reclen || receiver->reclen == (int)receiver->outfile.size() ) {
	    // just treat is as finished. Some mobile phones behave this way.
	    if( receiver->reclen ) {
		receiver->m_linkErrFinish = TRUE;
	    }
	    receiver->state = QObexReceiver::Finished; 
	} else {
	    receiver->state = QObexReceiver::Error;
	}
	receiver->finished = TRUE;
	// Disconnect transport:
	OBEX_TransportDisconnect( handle );
	break;

    default:
	break;
    }

}

void QObexReceiver::doPending()
{
    if ( state == Finished || state == Error ) {
	if ( !aborted ) {
	    deleteMeLater();
	    if ( state == Error ) {
		emit error();
	    }
	}
#ifdef QTOPIA_DEBUG_OBEX
	qDebug("emitting receiving( FALSE ) from doPending");
#endif
	emit receiving(FALSE);
    }
}


#include "qirserver.moc"

QIrServer::QIrServer( QObject *parent, const char *name )
    :QObject( parent, name )
{
    QObexServer* server = new QObexServer( this );
    connect(server, SIGNAL(receiving(bool)), this, SLOT(receiving(bool)) );
    bip = rip = FALSE;

    clean();
    needClean = FALSE;
}

QIrServer::~QIrServer()
{
}

QString QIrServer::landingZone( bool inbound )
{
    static QString in;
    static QString out;

    if ( !in.isEmpty() ) return inbound ? in : out;
    in = Global::homeDirPath() + "/tmp/obex/in";
    out = Global::homeDirPath() + "/tmp/obex/out";
    return inbound ? in : out;
}

void QIrServer::beam( const QString& filename, const QString& mimetype )
{
    if ( bip ) {
	qDebug("QIrServer not ready, beaming disallowed");
	emit beamError();
	return;
    }
#ifdef QTOPIA_DEBUG_OBEX
    qDebug("beaming %s (type %s)", filename.data(), mimetype.latin1() );
#endif
    bip = TRUE;
    
    QObexSender *sender = new QObexSender( this );

    connect( this, SIGNAL(abortBeam()), sender, SLOT(abort()) );
    
    connect( sender, SIGNAL(done()), this, SLOT(bDone()) );
    connect( sender, SIGNAL(error()), this, SLOT(bError()) );
    connect( sender, SIGNAL(statusMsg(const QString&)), this, SIGNAL(statusMsg(const QString&)) );
    connect( sender, SIGNAL(progress(int)), this, SIGNAL( progressSend(int) ) );
    
    sender->beam( filename, mimetype );
    
    //fire-and forget, auto-cleanup
}

void QIrServer::receiving(bool b)
{
    if ( rip != b ) {
	rip = b;

	if ( rip ) {
	    emit receiveInit();
	} else {
	    emit receiveDone();
	}
#ifdef QTOPIA_DEBUG_OBEX
	    qDebug("QIrServer::receiving(%s)", b ? "TRUE" : "FALSE");
#endif
	if( b && needClean ) {
	    clean();
	    needClean = FALSE;
	}
	if( !b ) // after change to not receiving, need to clean next time receiving
	    needClean = TRUE;
    }
}

void QIrServer::rError()
{
    rip = FALSE;
    emit receiveError();
}

void QIrServer::bDone()
{
    bip = FALSE;
    emit beamDone();
}

void QIrServer::bError()
{
    bip = FALSE;
    emit beamError();
}

void QIrServer::cancelBeam()
{
    // internal signal
    emit abortBeam();
    
    //external (this goes to qir)
    bError();
}

void QIrServer::cancelReceive()
{
    // internal signal
    emit abortReceive();
    
    //external (this goes to qir)
    rError();
}

void QIrServer::setReceivingEnabled( bool )
{
    //XXX

}
