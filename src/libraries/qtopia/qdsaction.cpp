/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// Local includes
#include "qds_p.h"
#include "qdsaction.h"
#include "qdsaction_p.h"
#include "qdsactionrequest.h"

// Qt includes
#include <QTimer>

// Qtopia includes
#include <QtopiaApplication>
#include <QtopiaServiceRequest>
#include <qtopialog.h>

// ============================================================================
//
//  QDSActionPrivate
//
// ============================================================================

// Created with uuidgen
QUniqueIdGenerator QDSActionPrivate::mIdGen(
    "ccd2d4cd-ecca-4b6c-96ef-a87645f5c52c" ); // No tr

QDSActionPrivate::QDSActionPrivate()
:   QObject(),
    mId(),
    mServiceInfo(),
    mResponseChannel( 0 ),
    mTimer( 0 ),
    mEventLoop( 0 ),
    mResponseData(),
    mErrorMsg(),
    mResponseCode( QDSAction::Invalid )
{
}

QDSActionPrivate::QDSActionPrivate( const QDSActionPrivate& other )
:   QObject(),
    mId( other.mId ),
    mServiceInfo( other.mServiceInfo ),
    mResponseChannel( 0 ),
    mTimer( 0 ),
    mEventLoop( 0 ),
    mResponseData( other.mResponseData ),
    mErrorMsg( other.mErrorMsg ),
    mResponseCode( QDSAction::Invalid )
{
}

QDSActionPrivate::QDSActionPrivate( const QString& name,
                                    const QString& service )
:   QObject(),
    mId(),
    mServiceInfo( name, service ),
    mResponseChannel( 0 ),
    mTimer( 0 ),
    mEventLoop( 0 ),
    mResponseData(),
    mErrorMsg(),
    mResponseCode( QDSAction::Invalid )
{
    mId = mIdGen.createUniqueId();
}

QDSActionPrivate::QDSActionPrivate( const QDSServiceInfo& serviceInfo )
:   QObject(),
    mId(),
    mServiceInfo( serviceInfo ),
    mResponseChannel( 0 ),
    mTimer( 0 ),
    mEventLoop( 0 ),
    mResponseData(),
    mErrorMsg(),
    mResponseCode( QDSAction::Invalid )
{
}

QDSActionPrivate::~QDSActionPrivate()
{
    disconnectFromChannel();
}

bool QDSActionPrivate::requestActive()
{
    if ( ( mTimer != 0 ) && ( mTimer->isActive() ) )
        return true;

    return false;
}

void QDSActionPrivate::emitRequest()
{
    connectToChannel();
    startTimer();

    QtopiaServiceRequest serviceRequest(
        mServiceInfo.serviceId(),
        mServiceInfo.name() + "(QDSActionRequest)" ); // No tr

    serviceRequest << QDSActionRequest( mServiceInfo, responseChannel() );
    serviceRequest.send();
}

void QDSActionPrivate::emitRequest( const QDSData& requestData,
                                    const QByteArray& auxiliary )
{
    connectToChannel();
    startTimer();

    QtopiaServiceRequest serviceRequest(
        mServiceInfo.serviceId(),
        mServiceInfo.name() + "(QDSActionRequest)" ); // No tr

    serviceRequest << QDSActionRequest( mServiceInfo,
                                        requestData,
                                        responseChannel(),
                                        auxiliary );
    serviceRequest.send();
}

void QDSActionPrivate::heartbeatSlot()
{
    mTimer->stop();
    mTimer->start( QDS::REQUEST_TIMEOUT );
}

void QDSActionPrivate::responseSlot()
{
    mTimer->stop();

    emit response( mId );

    mResponseCode = QDSAction::Complete;
    if ( mEventLoop != 0 )
        mEventLoop->exit();
}

void QDSActionPrivate::responseSlot( const QDSData& responseData )
{
    mTimer->stop();

    mResponseData = responseData;
    emit response( mId, responseData );

    mResponseCode = QDSAction::CompleteData;
    if ( mEventLoop != 0 )
        mEventLoop->exit();
}

void QDSActionPrivate::requestTimeoutSlot()
{
    mTimer->stop();

    mResponseCode = QDSAction::Error;
    mErrorMsg = tr( "timeout" );
    emit error( mId, mErrorMsg );

    if ( mEventLoop != 0 )
        mEventLoop->exit();
}

void QDSActionPrivate::errorSlot( const QString& message )
{
    mTimer->stop();

    mResponseCode = QDSAction::Error;
    mErrorMsg = message;
    emit error( mId, message );

    if ( mEventLoop != 0 )
        mEventLoop->exit();
}

void QDSActionPrivate::connectToChannel()
{
    // Connect slots to response messages
    if ( mResponseChannel == 0 ) {
        mResponseChannel = new QtopiaIpcAdaptor( responseChannel() );

        QtopiaIpcAdaptor::connect(
            mResponseChannel,
            MESSAGE( heartbeat() ),
            this,
            SLOT( heartbeatSlot() ),
            QtopiaIpcAdaptor::SenderIsChannel );

        QtopiaIpcAdaptor::connect(
            mResponseChannel,
            MESSAGE( response() ),
            this,
            SLOT( responseSlot() ),
            QtopiaIpcAdaptor::SenderIsChannel );

        QtopiaIpcAdaptor::connect(
            mResponseChannel,
            MESSAGE( response( const QDSData& ) ),
            this,
            SLOT( responseSlot( const QDSData& ) ),
            QtopiaIpcAdaptor::SenderIsChannel );

        QtopiaIpcAdaptor::connect(
            mResponseChannel,
            MESSAGE( error( const QString& ) ),
            this,
            SLOT( errorSlot( const QString& ) ),
            QtopiaIpcAdaptor::SenderIsChannel );
    }
}

void QDSActionPrivate::disconnectFromChannel()
{
    delete mResponseChannel;
    mResponseChannel = 0;
}

void QDSActionPrivate::startTimer()
{
    if ( mTimer == 0 ) {
        mTimer = new QTimer( this );
        mTimer->setSingleShot( true );
        connect( mTimer,
                 SIGNAL( timeout() ),
                 this,
                 SLOT( requestTimeoutSlot() ) );
    }

    mTimer->start( QDS::REQUEST_TIMEOUT );
}

QString QDSActionPrivate::responseChannel()
{
    QString channel = "QPE/QDSResponse/";
    channel += mId.toString();

    return channel;
}

void QDSActionPrivate::reset()
{
    mErrorMsg = QString();
    mResponseCode = QDSAction::Invalid;
    mResponseData = QDSData();
    mEventLoop = 0;

    if ( mTimer != 0 )
        mTimer->stop();
}

void QDSActionPrivate::connectToAction( QDSAction* action )
{
    connect( this,
             SIGNAL( response( const QLocalUniqueId& ) ),
             action,
             SIGNAL( response( const QLocalUniqueId& ) ) );

    connect( this,
             SIGNAL( response( const QLocalUniqueId&, const QDSData& ) ),
             action,
             SIGNAL( response( const QLocalUniqueId&, const QDSData& ) ) );

    connect( this,
             SIGNAL( error( const QLocalUniqueId&, const QString& ) ),
             action,
             SIGNAL( error( const QLocalUniqueId&, const QString& ) ) );
}

// ============================================================================
//
//  QDSAction
//
// ============================================================================

/*!
    \class QDSAction
    \brief The QDSAction class provides an interface for requesting QDS
    services.

    Applications use the QDSAction class to make a request for a QDS service.
    The request can be made either synchronously or asynchronously by using
    either QDSData::exec() or QDSData::invoke().

    \sa QDSServiceInfo

    \ingroup ipc
*/

/*!
    \fn void QDSAction::response( const QLocalUniqueId& actionId )

    Signal that is emitted when a response is received from the service provider
    for action identified by \a actionId.
*/

/*!
    \fn void QDSAction::response( const QLocalUniqueId& actionId, const QDSData&
    responseData )

    Signal that is emitted when a response is received from the service provider,
    which contains the response data \a responseData for action identified by
    \a actionId.
*/

/*!
    \fn void QDSAction::error( const QLocalUniqueId& actionId, const QString& message )

    Signal that is emitted when a error message \a message is received for the
    action identified by \a actionId.
*/

/*!
    \enum QDSAction::ResponseCode
    Response code for synchronous requests.

    \value Invalid Response code has not been set.
    \value Complete The request was processed correctly.
    \value CompleteData The request was processed correctly, and response data was
    received.
    \value Error An error occured, use QDSAction::errorMessage() to view the
           error message.
*/

/*!
    Default constructor which creates an empty QDSAction and attaches it to
    \a parent.
*/
QDSAction::QDSAction( QObject* parent )
:   QObject( parent ),
    d( 0 )
{
    // Create d pointer
    d = new QDSActionPrivate();
    d->connectToAction( this );
}

/*!
    Copy constructor which creates a deep copy of \a other
*/
QDSAction::QDSAction( const QDSAction& other )
:   QObject(),
    d( 0 )
{
    // Create d pointer and copy members
    d = new QDSActionPrivate( *( other.d ) );
    d->connectToAction( this );
}

/*!
    Constructor for the QDS service \a name and Qtopia service \a service.
    The action is attached to \a parent.
*/
QDSAction::QDSAction( const QString& name,
                      const QString& service,
                      QObject* parent )
:   QObject( parent ),
    d( 0 )
{
    // Create d pointer
    d = new QDSActionPrivate( name, service );
    d->connectToAction( this );
}

/*!
    Constructor which accesses the service in \a serviceInfo and attaches to
    \a parent.
*/
QDSAction::QDSAction( const QDSServiceInfo& serviceInfo, QObject* parent )
:   QObject( parent ),
    d( 0 )
{
    // Create d pointer
    d = new QDSActionPrivate( serviceInfo );
    d->connectToAction( this );
}

/*!
    Destructor.
*/
QDSAction::~QDSAction()
{
    if ( d->requestActive() )
        qLog(DataSharing) << "QDSAction object destroyed while request active";

    delete d;
}

/*!
    Assignment operator which deep copies \a other.
*/
const QDSAction& QDSAction::operator=( const QDSAction& other )
{
    d->mId = other.id();
    d->mServiceInfo = other.serviceInfo();
    d->mResponseData = other.responseData();
    d->mErrorMsg = other.errorMessage();
    d->mResponseCode = other.d->mResponseCode;

    // Don't copy mResponseChannel, mTimer or mEventLoop as they are
    // specific to an instance/operation

    return *this;
}

/*!
    Determines if the QDSAction object represents a valid QDS service.
    The requirements of a valid QDS service are discussed in QDSServiceInfo.
*/
bool QDSAction::isValid() const
{
    return d->mServiceInfo.isValid();
}

/*!
    Determines if the QDSAction object represents an available QDS service.
    The requirements of an available QDS service are discussed in QDSServiceInfo.
*/
bool QDSAction::isAvailable() const
{
    return d->mServiceInfo.isAvailable();
}

/*!
    Determines if the request is still being processed by the QDS service
    provider.
*/
bool QDSAction::isActive() const
{
    return d->requestActive();
}

/*!
    Returns the unique identifier for the QDSAction
*/
QLocalUniqueId QDSAction::id() const
{
    return d->mId;
}

/*!
    Returns QDSServiceInfo object which describes the QDS service being
    utilised.
*/
const QDSServiceInfo& QDSAction::serviceInfo() const
{
    return d->mServiceInfo;
}

/*!
    Returns the QDSData which was generated by the request.
*/
QDSData QDSAction::responseData() const
{
    return d->mResponseData;
}

/*!
    Returns any error message that was generated during the request. Errors
    are reported from either QDSAction or the provider application, and are
    reported through QDSData::error() or the return value of
    QDSData::exec(). If no error has been reported QString() will be
    returned.
*/
QString QDSAction::errorMessage() const
{
    return d->mErrorMsg;
}

/*!
    Initiates the QDS service request with \a requestData asynchronously. Depending
    on the outcome of the request, the response from the QDS service provider
    is given by one of QDSData::response() or QDSData::error() signals.

    The request may also contain \a auxiliary data for supplementary
    information which may be  required for the request but does not
    conceptionally belong to \a requestData.

    This method should only be used for QDS services which have request data, as
    discussed in QDSServiceInfo, otherwise an error will be generated.
*/
bool QDSAction::invoke( const QDSData &requestData, const QByteArray& auxiliary )
{
    if ( !isValid() ) {
        d->mErrorMsg = tr( "invalid service" );
        return false;
    }

    if ( !isAvailable() ) {
        d->mErrorMsg = tr( "unavailable service" );
        return false;
    }

    if ( !d->mServiceInfo.supportsRequestDataType( requestData.type() ) ) {
        d->mErrorMsg = tr( "incorrect data type" );
        return false;
    }

    d->reset();
    d->emitRequest( requestData, auxiliary );

    return true;
}

/*!
    Initiates the QDS service request asynchronously. Depending on the outcome
    of the request, the response from the QDS service provider is given by one
    of QDSData::response() or QDSData::error() signals.

    This method should only be used for QDS services which don't have request
    data, as discussed in QDSServiceInfo, otherwise an error will be generated.
*/
bool QDSAction::invoke()
{
    if ( !isValid() ) {
        d->mErrorMsg = tr( "invalid service" );
        return false;
    }

    if ( !isAvailable() ) {
        d->mErrorMsg = tr( "unavailable service" );
        return false;
    }

    if ( !d->mServiceInfo.supportsRequestDataType( QMimeType(QString()) ) ) {
        d->mErrorMsg = tr( "expecting data" );
        return false;
    }

    d->reset();
    d->emitRequest();

    return true;
}

/*!
    Performs the QDS service request synchronously. The return value provides
    the result of the request, see QDSAction::ResponseCode. Beware, this call
    will block until the response has been received from the QDS service
    or the request times out.

    This method should only be used for QDS services which don't have request
    data, as discussed in QDSServiceInfo, otherwise an error will be
    generated.
*/
int QDSAction::exec()
{
    if ( !isValid() ) {
        d->mErrorMsg = tr( "invalid service" );
        return QDSAction::Error;
    }

    if ( !isAvailable() ) {
        d->mErrorMsg = tr( "unavailable service" );
        return QDSAction::Error;
    }

    if ( !d->mServiceInfo.supportsRequestDataType( QMimeType(QString()) ) ) {
        d->mErrorMsg = tr( "expecting data" );
        return QDSAction::Error;
    }

    if (d->mEventLoop) {
        d->mErrorMsg = tr( "QDSAction::exec: Recursive call detected" );
        qLog(DataSharing) << d->mErrorMsg.toLatin1();
        return QDSAction::Error;
    }

    invoke();

    QEventLoop eventLoop;
    d->mEventLoop = &eventLoop;
    (void) eventLoop.exec();
    d->mEventLoop = 0;

    return d->mResponseCode;
}

/*!
    Performs the QDS service request with \a requestData synchronously. The return
    value provides the result of the request, see QDSAction::ResponseCode.
    Beware, this call will block until the response has been received from the
    QDS service or the request times out.

    The request may also contain \a auxiliary data for supplementary
    information which may be  required for the request but does not
    conceptionally belong to \a requestData.

    This method should only be used for QDS services which have request data,
    as discussed in QDSServiceInfo, otherwise an error will be generated.
*/
int QDSAction::exec( const QDSData& requestData, const QByteArray& auxiliary )
{
    if ( !isValid() ) {
        d->mErrorMsg = tr( "invalid service" );
        return QDSAction::Error;
    }

    if ( !isAvailable() ) {
        d->mErrorMsg = tr( "unavailable service" );
        return QDSAction::Error;
    }

    if ( !d->mServiceInfo.supportsRequestDataType( requestData.type() ) ) {
        d->mErrorMsg = tr( "incorrect data type" );
        return QDSAction::Error;
    }

    if (d->mEventLoop) {
        d->mErrorMsg = tr( "QDSAction::exec: Recursive call detected" );
        qLog(DataSharing) << d->mErrorMsg.toLatin1();
        return QDSAction::Error;
    }

    invoke( requestData, auxiliary );

    QEventLoop eventLoop;
    d->mEventLoop = &eventLoop;
    (void) eventLoop.exec();
    d->mEventLoop = 0;

    return d->mResponseCode;
}


