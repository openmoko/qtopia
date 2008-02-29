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

#include <qsocketnotifier.h>
#include <qfile.h>
#include <qtimer.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

extern "C" { 
#include "openobex/obex.h"
}

//#include <qmessagebox.h>

class QObexBase :public QObject
{
    Q_OBJECT;
public:
    QObexBase( QObject *parent = 0, const char *name = 0 );
    ~QObexBase();

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



QObexBase::QObexBase( QObject *parent = 0, const char *name = 0 )
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
    void receiving( int size, const QString & name, const QString & mime );
    void progress( int size );
    void received( const QString & name, const QString & mime );

private:
    friend void qobex_receiver_callback(obex_t *handle, obex_object_t *object, gint mode, gint event, gint obex_cmd, gint obex_rsp);

    enum State { Init, Connecting, Receiving,   
		 Disconnecting, Finished, Error, Aborted };

    void readData( obex_object_t *);

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
	deleteMeLater();

    }
}




static void qobex_sender_callback(obex_t *handle, obex_object_t *obj, gint /*mode*/, gint event, gint /*obex_cmd*/, gint /*obex_rsp*/)
{
    //qDebug( "qobex_sender_callback %p, %p, %d, event %x, cmd %x, rsp %x",
    //    handle, obj, mode, event, obex_cmd, obex_rsp );

    QObexSender *sender = (QObexSender*)OBEX_GetUserData( handle );


    switch (event) {
    case OBEX_EV_REQDONE:
	sender->finished = TRUE;
	emit sender->done();
	
	break;
	
    case OBEX_EV_LINKERR:
	sender->finished = TRUE;
	sender->state = QObexSender::Error;
	emit sender->done();
	break;

    case OBEX_EV_PROGRESS:
	// report progress?
	break;

	
    case OBEX_EV_STREAMEMPTY:
	// when streaming: add more
	sender->feedStream( obj );
	break;
	
    default:
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
    qDebug( "beaming %s", filename.latin1() );

    file_to_send = filename;
    mime_to_send = mimetype;

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
	qDebug( "Connection attempt %d failed. Retrying...",
		connectCount );
	QTimer::singleShot( 500, this, SLOT(tryConnecting()) );
	return;
    }
    
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
    QObexReceiver *receiver = new QObexReceiver( handle, this );
    connect( receiver, SIGNAL(receiving(int, const QString&, const QString&)),
	     parent(), SIGNAL(receiving(int, const QString&, const QString&)) );
    connect( receiver, SIGNAL(progress(int)),
	     parent(), SIGNAL(progress(int)) );
    connect( receiver, SIGNAL(received(const QString&, const QString&)),
	     parent(), SIGNAL(received(const QString&, const QString&)) );

    connect( parent(), SIGNAL(abort()), receiver, SLOT(abort()) );


}



static void qobex_server_callback(obex_t *handle, obex_object_t * /*object*/, gint /*mode*/, gint event, gint /*obex_cmd*/, gint /*obex_rsp*/)
{

    //    qDebug( "qobex_server_callback %p event %x cmd %x rsp %x", object, event, obex_cmd, obex_rsp );

    QObexServer* server = 
	(QObexServer*)OBEX_GetUserData( handle );

    switch (event)        {
    case OBEX_EV_ACCEPTHINT:
	{
	    qDebug( "#####------ OBEX_EV_ACCEPTHINT ------########" );

	    server->spawnReceiver( handle );
	}
	break;
    default:
	qFatal( " Unexpected event in server" );
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
    qDebug( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~QObexReceiver()" );
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
	    //qDebug( "******** Got length %d", hv.bq4 );
	    reclen = hv.bq4;
	    break;

	case OBEX_HDR_NAME:
	    filename = getString(hv.bs, hv_size);
	    /*
		qDebug( "******** Got name %s (%d)", 
			getString(hv.bs, hv_size).latin1(), hv_size );
	    */
	    break;


	case HDR_DESCRIPTION:
	    /*
		qDebug( "******** Got description %s (%d)", 
			getString(hv.bs, hv_size).latin1(), hv_size );
	    */
	    break;



	case OBEX_HDR_TYPE:
	    mimetype = QString::fromLatin1( (const char*)hv.bs, hv_size );
	    /*
		qDebug( "******** Got type %s", mimetype.latin1() );
	    */
	    break;


	default:
	    /*
	    if ( hi < 0x40 ) //unicode header
		qDebug( "******** Got header 0x%x %s (%d)", hi, 
			getString(hv.bs, hv_size).latin1(), hv_size );
	    else if ( (hi & 0xc0) == 0xc0 ) // quantity [eg. cf = PalmOS Creator ID]
		qDebug( "******** Got header 0x%x (%d)", hi, hv.bq4 );
	    else
		qDebug( "******** Got header 0x%x (size=%d)", hi, hv_size );
	    */
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
	emit progress( reclen ? reclen : outfile.at() );
    } else if ( len == 0 ) {
	emit progress( reclen ? reclen : outfile.at() );
	outfile.close();

	emit received( outfile.name(), mimetype );
    } else {
	qWarning( "ERROR reading stream" );
	//emit error();
    }
    finished = ( len <= 0 );
}

static void qobex_receiver_callback(obex_t *handle, obex_object_t *object, gint /*mode*/, gint event, gint obex_cmd, gint obex_rsp)
{

       qDebug( "qobex_receiver_callback %p event %x cmd %x rsp %x", object, event, obex_cmd, obex_rsp );


    QObexReceiver* receiver = 
	(QObexReceiver*)OBEX_GetUserData( handle );

    switch (event)        {

    case OBEX_EV_STREAMAVAIL:
	receiver->readData( object );
	break;
    case OBEX_EV_PROGRESS:
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
    if ( state == Finished )
	deleteMeLater();
}


#include "qirserver.moc"

QIrServer::QIrServer( QObject *parent = 0, const char *name = 0 )
    :QObject( parent, name )
{
    (void)new QObexServer( this );
}

void QIrServer::beam( const QString& filename, const QString& mimetype )
{
    QObexSender *sender = new QObexSender( this );
    sender->beam( filename, mimetype );

    connect( this, SIGNAL(abort()), sender, SLOT(abort()) );
    connect( sender, SIGNAL(done()), this, SIGNAL(done()) );

    //fire-and forget, auto-cleanup
}

void QIrServer::cancel()
{
    emit abort();
}

void QIrServer::setReceivingEnabled( bool )
{
    //XXX

}
