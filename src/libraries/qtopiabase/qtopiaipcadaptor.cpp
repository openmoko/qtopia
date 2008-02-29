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

#include <qtopiaipcmarshal.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaservices.h>
#include <qtopiaipcenvelope.h>
#include <qsignalintercepter.h>
#include <qslotinvoker.h>
#include <qtopiachannel.h>
#include <qapplication.h>
#include <qmap.h>
#include <qset.h>
#include <qtimer.h>
#include <qmetaobject.h>
#include <QDebug>

/*!
    \class QtopiaIpcAdaptor
    \ingroup ipc
    \brief The QtopiaIpcAdaptor class provides an interface to messages on a Qtopia IPC channel
    which simplifies remote signal and slot invocations

    The QtopiaIpcAdaptor class provides an interface for sending messages on a
    Qtopia IPC channel which simplifies remote signal and slot invocations.

    \sa QtopiaAbstractService, QtopiaIpcEnvelope
*/

/*!
    \enum QtopiaIpcAdaptor::ChannelSelector
    Select which of the sender or receiver in QtopiaIpcAdaptor::connect is the channel.

    \value AutoDetect Automatically detect from the arguments (the default).
    \value SenderIsChannel The sender is the channel, so the receiver is
           treated as local even if it is an instance of QtopiaIpcAdaptor.
           This is normally only needed if both sender and receiver are
           instances of QtopiaIpcAdaptor.
    \value ReceiverIsChannel The receiver is the channel, so the sender
           is treated as local even if it is an instance of QtopiaIpcAdaptor.
           This is normally only needed if both sender and receiver are
           instances of QtopiaIpcAdaptor.
*/

class QtopiaIpcAdaptorPrivate
{
public:
    QtopiaIpcAdaptorPrivate( const QString& channelName )
        : m_ref(1)
    {
        this->channelName = channelName;
        this->connected = false;
        this->publishedTo = 0;
    }

    ~QtopiaIpcAdaptorPrivate()
    {
        qDeleteAll(invokers);
    }

    QString channelName;
    bool connected;
    const QMetaObject *publishedTo;
    QMultiMap< QString, QSlotInvoker * > invokers;
    QSet<QString> intercepted;

    void AddRef() {
        Q_ASSERT(m_ref);
        ++m_ref;
    }
    void Release() {
        Q_ASSERT(m_ref);
        --m_ref;
        if(!m_ref)
            delete this;
    }
private:
    int m_ref;
};

class QtopiaIpcSignalIntercepter : public QSignalIntercepter
{
    // Do not put Q_OBJECT here.
public:
    QtopiaIpcSignalIntercepter( QObject *sender, const QByteArray& signal,
                           QtopiaIpcAdaptor *parent, const QString& msg )
        : QSignalIntercepter( sender, signal, parent )
    {
        this->parent = parent;
        this->msg = msg;
    }

protected:
    void activated( const QList<QVariant>& args );

private:
    QtopiaIpcAdaptor *parent;
    QString msg;
};

void QtopiaIpcSignalIntercepter::activated( const QList<QVariant>& args )
{
    parent->sendMessage( msg, args );
}

/*!
    Construct a Qtopia IPC message object for \a channel and attach it to \a parent.
    If \a channel is empty, then messages are taken from the application's
    \c{appMessage} channel.
*/
QtopiaIpcAdaptor::QtopiaIpcAdaptor( const QString& channel, QObject *parent )
    : QObject( parent )
{
    d = new QtopiaIpcAdaptorPrivate( channel );
}

/*!
    Destroy this Qtopia IPC messaging object.
*/
QtopiaIpcAdaptor::~QtopiaIpcAdaptor()
{
    d->Release();
    d = 0;
}

/*!
    Connect \a signal on \a sender to \a member on \a receiver.
    If either \a sender or \a receiver are instances of
    QtopiaIpcAdaptor, this function will arrange for the signal
    to be delivered over a Qtopia IPC channel.  If both \a sender and
    \a receiver are local, this function is identical
    to QObject::connect().

    The \a selector argument allows the caller to disambiguate the
    case where both \a sender and \a receiver are instances of QtopiaIpcAdaptor.

    If the same signal is connected to same slot multiple times,
    then signal delivery will happen that many times.

    \sa QtopiaIpcAdaptor::ChannelSelector
*/
bool QtopiaIpcAdaptor::connect( QObject *sender, const QByteArray& signal,
                          QObject *receiver, const QByteArray& member,
                          QtopiaIpcAdaptor::ChannelSelector selector )
{
    QtopiaIpcAdaptor *senderProxy;
    QtopiaIpcAdaptor *receiverProxy;

    // Bail out if the parameters are invalid.
    if ( !sender || signal.isEmpty() || !receiver || member.isEmpty() )
        return false;

    // Resolve the objects to find the remote proxies.
    senderProxy = qobject_cast<QtopiaIpcAdaptor *>( sender );
    receiverProxy = qobject_cast<QtopiaIpcAdaptor *>( receiver );

    // Perform channel overrides as specified by "selector".
    if ( selector == SenderIsChannel )
        receiverProxy = 0;
    else if ( selector == ReceiverIsChannel )
        senderProxy = 0;

    // If neither has a proxy, then use a local connect.
    if ( !senderProxy && !receiverProxy ) {
        return QObject::connect( sender, signal, receiver, member );
    }

    // If both are remote proxies, then fail the request.
    if ( senderProxy && receiverProxy ) {
        qWarning( "QtopiaIpcAdaptor::connect: cannot determine the channel; "
                  "use SenderIsChannel or ReceiverIsChannel to disambiguate" );
        return false;
    }

    // Determine which direction the connect needs to happen in.
    if ( receiverProxy ) {
        // Connecting a local signal to a remote slot.
        return receiverProxy->connectLocalToRemote( sender, signal, member );
    } else {
        // Connecting a remote signal to a local slot.
        return senderProxy->connectRemoteToLocal( signal, receiver, member );
    }
}

/*!
    Publish the signal or slot called \a member on this object on
    the Qtopia IPC channel represented by this QtopiaIpcAdaptor.

    If \a member is a slot, then whenever an application sends a
    message to the channel with that name, the system will arrange
    for the slot to be invoked.

    If \a member is a signal, then whenever this object emits that
    signal, the system will arrange for a message with that name to
    be sent on the channel.

    Returns false if \a member does not refer to a valid signal or slot.
*/
bool QtopiaIpcAdaptor::publish( const QByteArray& member )
{
    if ( member.size() >= 1 && member[0] == '1' ) {
        // Exporting a slot.
        return connectRemoteToLocal( "2" + member.mid(1), this, member );
    } else {
        // Exporting a signal.
        return connectLocalToRemote( this, member, member );
    }
}

/*!
    \enum QtopiaIpcAdaptor::PublishType
    Type of members to publish via QtopiaIpcAdaptor.

    \value Signals Publish only signals.
    \value Slots Publish only public slots.
    \value SignalsAndSlots Publish both signals and public slots.
*/

/*!
    Publish all signals or public slots on this object within subclasses of
    QtopiaIpcAdaptor.  This is typically called from a subclass constructor.
    The \a type indicates if all signals, all public slots, or both, should
    be published.  Private and protected slots will never be published.
*/
void QtopiaIpcAdaptor::publishAll( QtopiaIpcAdaptor::PublishType type )
{
    const QMetaObject *meta = metaObject();
    if ( meta != d->publishedTo ) {
        int count = meta->methodCount();
        int index;
        if ( d->publishedTo )
            index = d->publishedTo->methodCount();
        else
            index = QtopiaIpcAdaptor::staticMetaObject.methodCount();
        for ( ; index < count; ++index ) {

            QMetaMethod method = meta->method( index );
            if ( method.methodType() == QMetaMethod::Slot &&
                 method.access() == QMetaMethod::Public &&
                 ( type == Slots || type == SignalsAndSlots ) ) {
                QByteArray name = method.signature();
                connectRemoteToLocal( "2" + name, this, "1" + name );
            } else if ( method.methodType() == QMetaMethod::Signal &&
                        ( type == Signals || type == SignalsAndSlots ) ) {
                QByteArray name = method.signature();
                name = "2" + name;
                connectLocalToRemote( this, name, name );
            }
        }
        d->publishedTo = meta;
    }
}

/*!
    Send a message on the Qtopia IPC channel which will cause the invocation
    of \a member on receiving objects.  The return value can be used
    to add arguments to the message before transmission.
*/
QtopiaIpcSendEnvelope QtopiaIpcAdaptor::send( const QByteArray& member )
{
    return QtopiaIpcSendEnvelope
        ( sendChannels( d->channelName ), memberToMessage( member ) );
}

/*!
    Send a message on the Qtopia IPC channel which will cause the invocation
    of the single-argument \a member on receiving objects, with the
    argument \a arg1.
*/
void QtopiaIpcAdaptor::send( const QByteArray& member, const QVariant &arg1 )
{
    QList<QVariant> args;
    args.append( arg1 );
    sendMessage( memberToMessage( member ), args );
}

/*!
    Send a message on the Qtopia IPC channel which will cause the invocation
    of the double-argument \a member on receiving objects, with the
    arguments \a arg1 and \a arg2.
*/
void QtopiaIpcAdaptor::send( const QByteArray& member, const QVariant &arg1, const QVariant &arg2 )
{
    QList<QVariant> args;
    args.append( arg1 );
    args.append( arg2 );
    sendMessage( memberToMessage( member ), args );
}

/*!
    Send a message on the Qtopia IPC channel which will cause the invocation
    of the triple-argument \a member on receiving objects, with the
    arguments \a arg1, \a arg2, and \a arg3.
*/
void QtopiaIpcAdaptor::send( const QByteArray& member, const QVariant &arg1,
                       const QVariant &arg2, const QVariant &arg3 )
{
    QList<QVariant> args;
    args.append( arg1 );
    args.append( arg2 );
    args.append( arg3 );
    sendMessage( memberToMessage( member ), args );
}

/*!
    Send a message on the Qtopia IPC channel which will cause the invocation
    of the multi-argument \a member on receiving objects, with the
    argument list \a args.
*/
void QtopiaIpcAdaptor::send( const QByteArray& member, const QList<QVariant>& args )
{
    sendMessage( memberToMessage( member ), args );
}

/*!
    Determine if the message on the Qtopia IPC channel corresponding to \a signal
    has been connected to a local slot.
*/
bool QtopiaIpcAdaptor::isConnected( const QByteArray& signal )
{
    return d->invokers.contains( memberToMessage( signal ) );
}

/*!
    Convert a signal or slot \a member name into a Qtopia IPC message name.
    The default implementation strips the signal or slot prefix number
    from \a member and then normalizes the name to convert types
    such as \c{const QString&} into QString.
*/
QString QtopiaIpcAdaptor::memberToMessage( const QByteArray& member )
{
    if ( member.size() >= 1 && member[0] >= '0' && member[0] <= '9' ) {
        return QString::fromLatin1
            ( QMetaObject::normalizedSignature( member.constData() + 1 ) );
    } else {
        return QString::fromLatin1( member.data(), member.size() );
    }
}

/*!
    Convert \a channel into a list of names to use for sending messages.
    The default implementation returns a list containing just \a channel.
*/
QStringList QtopiaIpcAdaptor::sendChannels( const QString& channel )
{
    QStringList list;
    list << channel;
    return list;
}

/*!
    Convert \a channel into a new name to use for receiving messages.
    The default implementation returns \a channel.
*/
QString QtopiaIpcAdaptor::receiveChannel( const QString& channel )
{
    return channel;
}

void QtopiaIpcAdaptor::received( const QString& msg, const QByteArray& data )
{
    QtopiaIpcAdaptorPrivate *priv = d;
    priv->AddRef();
    QMultiMap< QString, QSlotInvoker * >::ConstIterator iter;
    for ( iter = priv->invokers.find( msg );
          iter != priv->invokers.end() && iter.key() == msg; ++iter ) {
        QSlotInvoker *invoker = iter.value();
        int *params = invoker->parameterTypes();
        int numParams = invoker->parameterTypesCount();
        QDataStream stream( data );
        QList<QVariant> args;
        for ( int param = 0; param < numParams; ++param ) {
            if ( params[param] != QSignalIntercepter::QVariantId ) {
                QtopiaIpcAdaptorVariant temp;
                temp.load( stream, params[param] );
                args.append( temp );
            } else {
                // We need to handle QVariant specially because we actually
                // need the type header in this case.
                QVariant temp;
                stream >> temp;
                args.append( temp );
            }
        }
    #if !defined(QT_NO_EXCEPTIONS)
        try {
    #endif
            invoker->invoke( args );
    #if !defined(QT_NO_EXCEPTIONS)
        } catch ( ... ) {
        }
    #endif
    }

#if !defined(QTOPIA_DBUS_IPC)
    if ( msg == "__query_qcop_messages(QString,QString)" ) {
        // The "qcop" command-line tool is querying us to get a
        // list of messages that we can respond to.
        QDataStream stream( data );
        QString prefix;
        QString responseChannel;
        stream >> prefix;
        stream >> responseChannel;
        if ( !prefix.isEmpty() && prefix != memberToMessage("") )
            return;     // Query of some other service on this channel.
        QMultiMap< QString, QSlotInvoker * >::ConstIterator iter;
        for ( iter = priv->invokers.begin(); iter != priv->invokers.end(); ++iter ) {
            QtopiaIpcEnvelope env( responseChannel, "queryResponse(QString)" );
            env << iter.key();
        }
        QSet<QString>::ConstIterator iter2;
        for ( iter2 = priv->intercepted.begin(); iter2 != priv->intercepted.end();
              ++iter2 ) {
            QtopiaIpcEnvelope env( responseChannel, "queryResponse(QString)" );
            env << *iter2;
        }
    }
#endif
    priv->Release();
}

bool QtopiaIpcAdaptor::connectLocalToRemote
    ( QObject *sender, const QByteArray& signal, const QByteArray& member )
{
    QSignalIntercepter *intercepter;
    QString name = memberToMessage( member );
    intercepter = new QtopiaIpcSignalIntercepter( sender, signal, this, name );
    if ( ! intercepter->isValid() ) {
        delete intercepter;
        return false;
    }
    if ( ! d->intercepted.contains( name ) )
        d->intercepted += name;
    return true;
}

bool QtopiaIpcAdaptor::connectRemoteToLocal
    ( const QByteArray& signal, QObject *receiver, const QByteArray& member )
{
    // Make sure that we are actively monitoring the channel for messages.
    if ( ! d->connected ) {
        QString chan = receiveChannel( d->channelName );
        if ( chan.isEmpty() ) {
            QObject::connect
                ( qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
                this, SLOT(received(const QString&,const QByteArray&)) );
        } else {
            QtopiaChannel *channel = new QtopiaChannel( chan, this );
            QObject::connect
                ( channel, SIGNAL(received(const QString&,const QByteArray&)),
                  this, SLOT(received(const QString&,const QByteArray&)) );
        }
        d->connected = true;
    }

    // Create a slot invoker to handle executing the member when necessary.
    QSlotInvoker *invoker = new QSlotInvoker( receiver, member, 0 );
    if ( ! invoker->memberPresent() ) {
        delete invoker;
        return false;
    }
    d->invokers.insert( memberToMessage( signal ), invoker );
    return true;
}

void QtopiaIpcAdaptor::sendMessage
        ( const QString& msg, const QList<QVariant>& args )
{
    send( sendChannels( d->channelName ), msg, args );
}

void QtopiaIpcAdaptor::send( const QStringList& channels,
                             const QString& msg, const QList<QVariant>& args )
{
    QByteArray array;
    {
        QDataStream stream
            ( &array, QIODevice::WriteOnly | QIODevice::Append );
        QList<QVariant>::ConstIterator iter;
        if ( !msg.contains( "QVariant" ) ) {
            for ( iter = args.begin(); iter != args.end(); ++iter ) {
                QtopiaIpcAdaptorVariant copy( *iter );
                copy.save( stream );
            }
        } else {
            QByteArray name = msg.toLatin1();
            name = QMetaObject::normalizedSignature( name.constData() );
            int numParams = 0;
            int *params = QSignalIntercepter::connectionTypes
                    ( name, numParams );
            int index = 0;
            for ( iter = args.begin(); iter != args.end(); ++iter, ++index ) {
                if ( index < numParams &&
                     params[index] == QSignalIntercepter::QVariantId ) {
                    // We need to handle QVariant specially because we actually
                    // need the type header in this case.
                    stream << *iter;
                } else {
                    QtopiaIpcAdaptorVariant copy( *iter );
                    copy.save( stream );
                }
            }
            if ( params )
                qFree( params );
        }
        // Stream is flushed and closed at this point.
    }
    QStringList::ConstIterator iter;
    for ( iter = channels.begin(); iter != channels.end(); ++iter ) {
        QtopiaChannel::send( *iter, msg, array );
    }
}

/*!
    \class QtopiaIpcSendEnvelope
    \ingroup ipc
    \brief The QtopiaIpcSendEnvelope class provides a mechanism to send Qtopia IPC messages with an argument number of arguments.

    The QtopiaIpcSendEnvelope class provides a mechanism to send Qtopia IPC messages
    with an arbitrary number of arguments, in a similar fashion to
    QtopiaIpcEnvelope.  The QtopiaIpcAdaptor::send() method returns an instance
    of this class, as demonstrated in the following example:

    \code
    QtopiaIpcAdaptor *channel = ...;
    QtopiaIpcSendEnvelope env = channel->send( MESSAGE(foo(QString)) );
    env << "Hello";
    \endcode

    \sa QtopiaIpcAdaptor, QtopiaIpcEnvelope
*/

class QtopiaIpcSendEnvelopePrivate
{
public:
    QStringList channels;
    QString message;
    bool shouldBeSent;
    QList<QVariant> arguments;
};

QtopiaIpcSendEnvelope::QtopiaIpcSendEnvelope
        ( const QStringList& channels, const QString& message )
{
    d = new QtopiaIpcSendEnvelopePrivate();
    d->channels = channels;
    d->message = message;
    d->shouldBeSent = true;
}

/*!
    Construct an empty QtopiaIpcSendEnvelope.
*/
QtopiaIpcSendEnvelope::QtopiaIpcSendEnvelope()
{
    d = new QtopiaIpcSendEnvelopePrivate();
    d->shouldBeSent = false;
}

/*!
    Construct a copy of \a value.
*/
QtopiaIpcSendEnvelope::QtopiaIpcSendEnvelope( const QtopiaIpcSendEnvelope& value )
{
    d = new QtopiaIpcSendEnvelopePrivate();
    d->channels = value.d->channels;
    d->message = value.d->message;
    d->arguments = value.d->arguments;
    d->shouldBeSent = true;

    // If we make a copy of another object, that other object
    // must not be transmitted.  This typically happens when
    // we do the following:
    //
    //  QtopiaIpcSendEnvelope env = channel->send( MESSAGE(foo(QString)) );
    //  env << "Hello";
    //
    // The intermediate copies of the envelope, prior to the arguments
    // being added, must not be transmitted.  Only the final version is.
    value.d->shouldBeSent = false;
}

/*!
    Destroy this envelope object and send the message.
*/
QtopiaIpcSendEnvelope::~QtopiaIpcSendEnvelope()
{
    if (d->shouldBeSent)
        QtopiaIpcAdaptor::send( d->channels, d->message, d->arguments );
    delete d;
}

/*!
    Copy \a value into this object.
*/
QtopiaIpcSendEnvelope& QtopiaIpcSendEnvelope::operator=( const QtopiaIpcSendEnvelope& value )
{
    if ( &value == this )
        return *this;

    d->channels = value.d->channels;
    d->message = value.d->message;
    d->arguments = value.d->arguments;

    // Don't transmit the original copy.  See above for details.
    d->shouldBeSent = true;
    value.d->shouldBeSent = false;

    return *this;
}

/*!
    \fn QtopiaIpcSendEnvelope& QtopiaIpcSendEnvelope::operator<<( const char *value )

    \overload
    Add \a value to the arguments for this Qtopia IPC message.
*/

/*!
    \fn QtopiaIpcSendEnvelope& QtopiaIpcSendEnvelope::operator<<( const T &value )
    Add \a value to the arguments for this Qtopia IPC message.
 */

void QtopiaIpcSendEnvelope::addArgument( const QVariant& value )
{
    d->arguments.append( value );
}
