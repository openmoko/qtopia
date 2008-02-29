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
#include "qirserver.h"

#include <qtopia/mimetype.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/private/task.h>

#include <qsocketnotifier.h>
#include <qfile.h>
#include <qtimer.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

//#define QTOPIA_DEBUG_OBEX

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
    else if ( finished )
	doPending();
}

static void qobex_sender_callback(obex_t *handle, obex_object_t *obj, gint mode, gint event, gint obex_cmd, gint obex_rsp);

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
    friend void qobex_sender_callback(obex_t *handle, obex_object_t *obj, gint mode, gint event, gint obex_cmd, gint obex_rsp);

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

static void qobex_server_callback(obex_t *handle, obex_object_t *object, gint mode, gint event, gint obex_cmd, gint obex_rsp);

class QObexServer : public QObexBase
{
  Q_OBJECT;
public:
  QObexServer( QObject *parent = 0, const char *name = 0 );
  ~QObexServer();

protected:
    void doPending();

signals:
    void receiving(bool);
    void receiving( int size, const QString & name );
    void received( const QString & name );

private:
    friend void qobex_server_callback(obex_t *handle, obex_object_t *object, gint mode, gint event, gint obex_cmd, gint obex_rs);

    void spawnReceiver( obex_t *handle );

    enum State { Init, Connecting, Receiving,   
		 Disconnecting, Finished, Error, Aborted };

    State state;
};





static void qobex_receiver_callback(obex_t *handle, obex_object_t *object, gint mode, gint event, gint obex_cmd, gint obex_rsp);

class QObexReceiver : public QObexBase
{
    Q_OBJECT;
public:
    QObexReceiver( obex_t *handle, QObject *parent = 0, const char *name = 0 );
    ~QObexReceiver();

protected:
    void doPending();

signals:
    void receiving(bool);
    void receiving( int size, const QString & name, const QString & mime );
    void progress( int size );
    void received( const QString & name, const QString & mime );

private:
    friend void qobex_receiver_callback(obex_t *handle, obex_object_t *object, gint mode, gint event, gint obex_cmd, gint obex_rsp);

    enum State { Init, Connecting, Receiving,   
		 Disconnecting, Finished, Error, Aborted };

    void readData( obex_object_t *);
    void updateProgress( obex_object_t *);
    void getHeaders( obex_object_t *object );

    State state;

    QString filename;
    QString mimetype;
    int reclen;

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
    if ( file_being_sent->open(IO_ReadOnly) ) {

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

}




static bool transmitDone = FALSE;

void QObexSender::doPending()
{
    //qDebug( "QObexSender::doPending %d", state );

    if ( state == Connecting ) {
	//qDebug( "state Connecting" );
	// finished connecting; time to send

	putFile( file_to_send, mime_to_send );
	file_to_send = QString::null;

    } else if ( state == Streaming ) {
	//qDebug( "state Streaming" );
	//finished streaming

	obex_object_t *object = OBEX_ObjectNew(self, OBEX_CMD_DISCONNECT);
	process( object );
	state = Disconnecting;
    } else if ( state == Disconnecting ) {
	//qDebug( "state Disconnecting" );

	OBEX_TransportDisconnect(self);
	state = Init;
	emit done();
	deleteMeLater();

    }
}


static void qobex_sender_callback(obex_t *handle, obex_object_t *obj, gint /* mode */ , gint event, gint obex_cmd, gint /* obex_rsp */)
{
    QObexSender *sender = (QObexSender*)OBEX_GetUserData( handle );

//    qDebug( "qobex_sender_callback %p, %p, %p, %d, event %x, cmd %x, rsp %x",
//       sender, handle, obj, mode, event, obex_cmd, obex_rsp );

    switch (event) {
    case OBEX_EV_REQDONE:
	sender->finished = TRUE;
	if ( obex_cmd == OBEX_CMD_DISCONNECT ) {
	    transmitDone = TRUE;
	}
	// else if OBEX_CMD_CONNECT, we could give feedback
	break;

    case OBEX_EV_LINKERR:
	// sometime we get a link error after we believed the connection was done.  Ignore this
	// as emitting an error after done does not make sense
	if ( !transmitDone ) {
	    emit sender->error();
	    sender->abort();
	    sender->state = QObexSender::Error;
	} else {
	    sender->abort();
	    qDebug("QIrServer:: got link error after done signal, no external signals sent");
	}
	
	break;

    case OBEX_EV_PROGRESS:
	// report progress?
	sender->updateProgress( obj );
	break;

	
    case OBEX_EV_STREAMEMPTY:
	// when streaming: add more
	sender->feedStream( obj );
	break;
	
    default:
	qDebug("qobex_sender_callback:: did not recongnize event signal %d", event);
	break;
    }
}


QObexSender::QObexSender( QObject *parent, const char *name )
  :QObexBase( parent, name )
{

  self = OBEX_Init( OBEX_TRANS_IRDA, qobex_sender_callback, 0 );
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

    if ( aborted ) {
	deleteMeLater();
	return;
    }
	
    int retc = IrOBEX_TransportConnect(self, "OBEX");
    
    if ( retc < 0 ) {
	const int maxTry = 20;
	if ( connectCount > maxTry ) {
	    abort();
	    emit error();
	    deleteMeLater();
	} else {
	    QString str = tr("Beam failed (%1/%2). Retrying...","eg. 1/3").arg(connectCount).arg(maxTry);
	    emit statusMsg( str );
	    QTimer::singleShot( 500, this, SLOT(tryConnecting()) );
	}
	return;
    }

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

    if(OBEX_ServerRegister(self, "OBEX") < 0) {
	qWarning( "could not register server" );
	state = Error;
    } else {
      connectSocket();
    }
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
    emit receiving( TRUE );
    
    QObexReceiver *receiver = new QObexReceiver( handle, this );
    connect( receiver, SIGNAL(receiving(int, const QString&, const QString&)),
	     parent(), SIGNAL(receiving(int, const QString&, const QString&)) );
    connect( receiver, SIGNAL(progress(int)),
	     parent(), SIGNAL(progress(int)) );
    connect( receiver, SIGNAL(received(const QString&, const QString&)),
	     parent(), SIGNAL(received(const QString&, const QString&)) );
    
    connect( receiver, SIGNAL(receiving(bool)), parent(), SLOT(receiving(bool)) );

    connect( parent(), SIGNAL(abort()), receiver, SLOT(abort()) );
    connect(receiver, SIGNAL(error()),
	    parent(), SLOT(mError()) );
    
}



static void qobex_server_callback(obex_t *handle, obex_object_t * /*object*/, gint /*mode*/, gint event, gint /*obex_cmd*/, gint /*obex_rsp*/)
{

    //    qDebug( "qobex_server_callback %p event %x cmd %x rsp %x", object, event, obex_cmd, obex_rsp );

    QObexServer* server = 
	(QObexServer*)OBEX_GetUserData( handle );

    switch (event)        {
    case OBEX_EV_ACCEPTHINT:
	{
	    //qDebug( "#####------ OBEX_EV_ACCEPTHINT ------########" );

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
    guint32 hv_size;
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

void QObexReceiver::readData( obex_object_t *object )
{
    if ( filename.isEmpty() ) {
	getHeaders( object );


	int slash = filename.find( '/', -1 );
	if ( slash > 0 ) {
	    filename = filename.mid( slash+1 );
	}

	if ( mimetype.isNull() ) {
	    MimeType mt(filename);
	    mimetype = mt.id();
	}

	emit receiving( reclen, filename, mimetype );

	mkdir( "/tmp/obex", 0755 );
	system( "rm -rf /tmp/obex/*" );
	
	if ( filename.isEmpty() )
	    filename = "unnamed"; // No tr
	filename = "/tmp/obex/" + filename;
	
	outfile.setName( filename );
	outfile.open( IO_WriteOnly );

    }


    const uchar* buf;
    int len = OBEX_ObjectReadStream( self, object, &buf );

    if ( len > 0 ) {
	outfile.writeBlock( (const char*)buf, len );
    } else if ( len == 0 ) {
	outfile.close();

	emit received( outfile.name(), mimetype );
    } else {
	qWarning( "ERROR reading stream" );
	emit error();
    }
    finished = ( len <= 0 );
}

void QObexReceiver::updateProgress( obex_object_t * /* obj */)
{
    emit progress( outfile.size() );
}


static void qobex_receiver_callback(obex_t *handle, obex_object_t *object, gint /*mode*/, gint event, gint obex_cmd, gint /*obex_rsp*/ )
{
//    qDebug( "qobex_receiver_callback %p event %x cmd %x rsp %x", object, event, obex_cmd, obex_rsp );
    QObexReceiver* receiver = 
	(QObexReceiver*)OBEX_GetUserData( handle );

    switch (event)        {

    case OBEX_EV_STREAMAVAIL:
	receiver->readData( object );
	break;
    case OBEX_EV_PROGRESS:
	receiver->updateProgress( object );
	// report progress
	break;

    case OBEX_EV_REQ:
	/* An incoming request */
	switch(obex_cmd) {
	case OBEX_CMD_CONNECT:
	case OBEX_CMD_DISCONNECT:
	    /* Dont need to do anything here.
	       Response is already set to
	       success by OBEX_EV_REQHINT event */
	    break;
	case OBEX_CMD_PUT:
	    //We do it all in STREAMAVAIL 
	    //receiver->handlePut(object);
	    break;
	}                 
	break;

    case OBEX_EV_ACCEPTHINT:

	//qFatal( " OBEX_EV_ACCEPTHINT in receiver" );
	OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_IMPLEMENTED,
			  OBEX_RSP_NOT_IMPLEMENTED);
	break;

    case OBEX_EV_REQHINT:

	/* A new request is coming in */
	switch(obex_cmd) {
	    /* Accept some commands! */
	case OBEX_CMD_PUT:
	    OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
	//turn on streaming...
	    OBEX_ObjectReadStream(handle, object, NULL);
	    break;


	case OBEX_CMD_CONNECT:
	case OBEX_CMD_DISCONNECT:
	    OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
	    break;
                
	default:
	    /* Reject any other commands */                       
	    OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_IMPLEMENTED,
			      OBEX_RSP_NOT_IMPLEMENTED);
	    break;
                
	}
	break;

    case OBEX_EV_REQDONE:
	if(obex_cmd == OBEX_CMD_DISCONNECT) {

	    receiver->state = QObexReceiver::Finished;
	    receiver->finished = TRUE;
	    // Disconnect transport:
	    OBEX_TransportDisconnect( handle );
	}
	break;

    case OBEX_EV_LINKERR:
	// just treat is as finished. SOme mobile phones behave this way.
	    receiver->state = QObexReceiver::Finished;
	    receiver->finished = TRUE;
	    // Disconnect transport:
	    OBEX_TransportDisconnect( handle );
	/* Not good */
	break;

    default:
	break;
    }

}

void QObexReceiver::doPending()
{
    if ( state == Finished ) {
	deleteMeLater();
	emit receiving(FALSE);
    }
}


#include "qirserver.moc"

QIrServer::QIrServer( QObject *parent, const char *name )
    :QObject( parent, name )
{
    QObexServer *ob = new QObexServer( this );
    connect(ob, SIGNAL(receiving(bool)), this, SLOT(receiving(bool)) );

    _state = Ready;
}

QIrServer::~QIrServer()
{
}

void QIrServer::beam( const QString& filename, const QString& mimetype )
{
    if ( _state != Ready ) {
	qDebug("QIrServer not ready, beaming disallowed");
	emit beamError();
	return;
    }
//    qDebug("beaming %s", filename.data() );
    _state = Beaming;
    
    QObexSender *sender = new QObexSender( this );

    connect( this, SIGNAL(abort()), sender, SLOT(abort()) );
    
    connect( sender, SIGNAL(done()), this, SLOT(mDone()) );
    connect( sender, SIGNAL(error()), this, SLOT(mError()) );
    connect( sender, SIGNAL(statusMsg(const QString &)), this, SIGNAL(statusMsg(const QString &)) );
    connect( sender, SIGNAL(progress(int)), this, SIGNAL( progress(int) ) );
    
    sender->beam( filename, mimetype );
    
    //fire-and forget, auto-cleanup
}

void QIrServer::receiving(bool b)
{
    State old = _state;
    
    if ( b )
	_state = Receiving;
    else
	_state = Ready;
    
    // no nice way of initializing the first receive call.  avoid sending more than one receiveInit call
    if ( _state != old && _state == Receiving)
	emit receiveInit();
}

void QIrServer::mDone()
{
    _state = Ready;
    emit beamDone();
}

void QIrServer::mError()
{
    State old = _state;
    _state = Ready;
    
    if ( old == Beaming )
        emit beamError();
    else if ( old == Receiving )
	emit receiveError();
    else
	qDebug("QIrServer::got error after we though connection was completed");
}

void QIrServer::cancel()
{
    // internal signal
    emit abort();
    
    //external (this goes to qir)
    mError();
}

void QIrServer::setReceivingEnabled( bool )
{
    //XXX

}
