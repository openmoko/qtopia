/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifndef Q_QDOC
#include "qcopadaptor_qd.h"
#include <qtopiaipcmarshal.h>
#include "qcopenvelope_qd.h"
#include "qcopchannel_qd.h"
#include <QSignalIntercepter>
#include <QSlotInvoker>
#include <QApplication>
#include <QMetaObject>
#include <QMetaMethod>
#include <QSet>

/*
    \class QCopAdaptor
    \brief The QCopAdaptor class provides an interface to messages on a QCop channel
    which simplifies remote signal and slot invocations.

    Using this class, it is very easy to convert a signal emission into an IPC
    message on a channel, and to convert an IPC message on a channel into a
    slot invocation.  In the following example, when the signal \c{valueChanged(int)}
    is emitted from the object \c source, the IPC message \c{changeValue(int)} will
    be sent on the channel \c{QPE/Foo}:

    \code
    QCopAdaptor *adaptor = new QCopAdaptor("QPE/Foo");
    QCopAdaptor::connect
        (source, SIGNAL(valueChanged(int)), adaptor, MESSAGE(changeValue(int)));
    \endcode

    Note that we use QCopAdaptor::connect() to connect the signal to the
    IPC message, not QObject::connect().  A common error is to use \c{connect()}
    without qualifying it with \c{QCopAdaptor::} and picking up
    QObject::connect() by mistake.

    On the server side of an IPC protocol, the \c{changeValue(int)} message can
    be connected to the slot \c{setValue()} on \c dest:

    \code
    QCopAdaptor *adaptor = new QCopAdaptor("QPE/Foo");
    QCopAdaptor::connect
        (adaptor, MESSAGE(changeValue(int)), dest, SIGNAL(setValue(int)));
    \endcode

    Now, whenever the client emits the \c{valueChanged(int)} signal, the
    \c{setValue(int)} slot will be automatically invoked on the server side,
    with the \c int parameter passed as its argument.

    Only certain parameter types can be passed across an IPC boundary in this fashion.
    The type must be visible to QVariant as a meta-type.  Many simple built-in
    types are already visible; for user-defined types, use Q_DECLARE_USER_METATYPE().

    \sa QtopiaAbstractService, QCopEnvelope
*/

/*
    \enum QCopAdaptor::ChannelSelector
    Select which of the sender or receiver in QCopAdaptor::connect is the channel.

    \value AutoDetect Automatically detect from the arguments (the default).
    \value SenderIsChannel The sender is the channel, so the receiver is
           treated as local even if it is an instance of QCopAdaptor.
           This is normally only needed if both sender and receiver are
           instances of QCopAdaptor.
    \value ReceiverIsChannel The receiver is the channel, so the sender
           is treated as local even if it is an instance of QCopAdaptor.
           This is normally only needed if both sender and receiver are
           instances of QCopAdaptor.
*/

namespace qdsync {

class QCopAdaptorPrivate
{
public:
    QCopAdaptorPrivate( const QString &channelName )
        : m_ref(1)
    {
        this->channelName = channelName;
        this->connected = false;
        this->publishedTo = 0;
    }

    ~QCopAdaptorPrivate()
    {
        qDeleteAll(invokers);
    }

    QString channelName;
    bool connected;
    const QMetaObject *publishedTo;
    QMultiMap< QString, QSlotInvoker * > invokers;
    QSet<QString> intercepted;

    void AddRef()
    {
        Q_ASSERT(m_ref);
        ++m_ref;
    }
    void Release()
    {
        Q_ASSERT(m_ref);
        --m_ref;
        if(!m_ref)
            delete this;
    }
private:
    int m_ref;
};

class QCopSignalIntercepter : public QSignalIntercepter
{
    // Do not put Q_OBJECT here.
public:
    QCopSignalIntercepter( QObject *sender, const QByteArray &signal,
                           QCopAdaptor *parent, const QString &msg )
        : QSignalIntercepter( sender, signal, parent )
    {
        this->parent = parent;
        this->msg = msg;
    }

protected:
    void activated( const QList<QVariant> &args );

private:
    QCopAdaptor *parent;
    QString msg;
};

};

void QCopSignalIntercepter::activated( const QList<QVariant> &args )
{
    parent->sendMessage( msg, args );
}

/*
    Construct a Qtopia IPC message object for \a channel and attach it to \a parent.
    If \a channel is empty, then messages are taken from the application's
    \c{appMessage} channel.
*/
QCopAdaptor::QCopAdaptor( const QString &channel, QObject *parent )
    : QObject( parent )
{
    d = new QCopAdaptorPrivate( channel );
}

/*
    Destroy this Qtopia IPC messaging object.
*/
QCopAdaptor::~QCopAdaptor()
{
    d->Release();
    d = 0;
}

/*
    Connects \a signal on \a sender to \a member on \a receiver.  Returns true
    if the connection was possible; otherwise returns false.

    If either \a sender or \a receiver are instances of
    QCopAdaptor, this function will arrange for the signal
    to be delivered over a Qtopia IPC channel.  If both \a sender and
    \a receiver are local, this function is identical
    to QObject::connect().

    The \a selector argument allows the caller to disambiguate the
    case where both \a sender and \a receiver are instances of QCopAdaptor.

    If the same signal is connected to same slot multiple times,
    then signal delivery will happen that many times.

    \sa QCopAdaptor::ChannelSelector
*/
bool QCopAdaptor::connect( QObject *sender, const QByteArray &signal,
                          QObject *receiver, const QByteArray &member,
                          QCopAdaptor::ChannelSelector selector )
{
    QCopAdaptor *senderProxy;
    QCopAdaptor *receiverProxy;

    // Bail out if the parameters are invalid.
    if ( !sender || signal.isEmpty() || !receiver || member.isEmpty() )
        return false;

    // Resolve the objects to find the remote proxies.
    senderProxy = qobject_cast<QCopAdaptor *>( sender );
    receiverProxy = qobject_cast<QCopAdaptor *>( receiver );

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
        qWarning( "QCopAdaptor::connect: cannot determine the channel; "
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

/*
    Publishes the signal or slot called \a member on this object on
    the Qtopia IPC channel represented by this QCopAdaptor.

    If \a member is a slot, then whenever an application sends a
    message to the channel with that name, the system will arrange
    for the slot to be invoked.

    If \a member is a signal, then whenever this object emits that
    signal, the system will arrange for a message with that name to
    be sent on the channel.

    Returns false if \a member does not refer to a valid signal or slot.

    \sa publishAll()
*/
bool QCopAdaptor::publish( const QByteArray &member )
{
    if ( member.size() >= 1 && member[0] == '1' ) {
        // Exporting a slot.
        return connectRemoteToLocal( "2" + member.mid(1), this, member );
    } else {
        // Exporting a signal.
        return connectLocalToRemote( this, member, member );
    }
}

/*
    \enum QCopAdaptor::PublishType
    Type of members to publish via QCopAdaptor.

    \value Signals Publish only signals.
    \value Slots Publish only public slots.
    \value SignalsAndSlots Publish both signals and public slots.
*/

/*
    Publishes all signals or public slots on this object within subclasses of
    QCopAdaptor.  This is typically called from a subclass constructor.
    The \a type indicates if all signals, all public slots, or both, should
    be published.  Private and protected slots will never be published.

    \sa publish()
*/
void QCopAdaptor::publishAll( QCopAdaptor::PublishType type )
{
    const QMetaObject *meta = metaObject();
    if ( meta != d->publishedTo ) {
        int count = meta->methodCount();
        int index;
        if ( d->publishedTo )
            index = d->publishedTo->methodCount();
        else
            index = QCopAdaptor::staticMetaObject.methodCount();
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

#if 0
/*
    Sends a message on the Qtopia IPC channel which will cause the invocation
    of \a member on receiving objects.  The return value can be used
    to add arguments to the message before transmission.
*/
QCopSendEnvelope QCopAdaptor::send( const QByteArray &member )
{
    return QCopSendEnvelope
        ( sendChannels( d->channelName ), memberToMessage( member ) );
}
#endif

/*
    Sends a message on the Qtopia IPC channel which will cause the invocation
    of the multi-argument \a member on receiving objects, with the
    argument list \a args.
*/
void QCopAdaptor::send( const QByteArray &member, const QList<QVariant> &args )
{
    sendMessage( memberToMessage( member ), args );
}

/*
    Returns true if the message on the Qtopia IPC channel corresponding to \a signal
    has been connected to a local slot; otherwise returns false.
*/
bool QCopAdaptor::isConnected( const QByteArray &signal )
{
    return d->invokers.contains( memberToMessage( signal ) );
}

/*
    Converts a signal or slot \a member name into a Qtopia IPC message name.
    The default implementation strips the signal or slot prefix number
    from \a member and then normalizes the name to convert types
    such as \c{const QString&} into QString.
*/
QString QCopAdaptor::memberToMessage( const QByteArray &member )
{
    if ( member.size() >= 1 && member[0] >= '0' && member[0] <= '9' ) {
        return QString::fromLatin1
            ( QMetaObject::normalizedSignature( member.constData() + 1 ) );
    } else {
        return QString::fromLatin1( member.data(), member.size() );
    }
}

/*
    Converts \a channel into a list of names to use for sending messages.
    The default implementation returns a list containing just \a channel.
*/
QStringList QCopAdaptor::sendChannels( const QString &channel )
{
    QStringList list;
    list << channel;
    return list;
}

/*
    Converts \a channel into a new name to use for receiving messages.
    The default implementation returns \a channel.
*/
QString QCopAdaptor::receiveChannel( const QString &channel )
{
    return channel;
}

void QCopAdaptor::received( const QString &msg, const QByteArray &data )
{
    QCopAdaptorPrivate *priv = d;
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
            QCopEnvelope env( responseChannel, "queryResponse(QString)" );
            env << iter.key();
        }
        QSet<QString>::ConstIterator iter2;
        for ( iter2 = priv->intercepted.begin(); iter2 != priv->intercepted.end();
              ++iter2 ) {
            QCopEnvelope env( responseChannel, "queryResponse(QString)" );
            env << *iter2;
        }
    }
#endif
    priv->Release();
}

bool QCopAdaptor::connectLocalToRemote
    ( QObject *sender, const QByteArray &signal, const QByteArray &member )
{
    QSignalIntercepter *intercepter;
    QString name = memberToMessage( member );
    intercepter = new QCopSignalIntercepter( sender, signal, this, name );
    if ( ! intercepter->isValid() ) {
        delete intercepter;
        return false;
    }
    if ( ! d->intercepted.contains( name ) )
        d->intercepted += name;
    return true;
}

bool QCopAdaptor::connectRemoteToLocal
    ( const QByteArray &signal, QObject *receiver, const QByteArray &member )
{
    // Make sure that we are actively monitoring the channel for messages.
    if ( ! d->connected ) {
        QString chan = receiveChannel( d->channelName );
        if ( chan.isEmpty() ) {
            QObject::connect
                ( qApp, SIGNAL(appMessage(QString,QByteArray)),
                this, SLOT(received(QString,QByteArray)) );
        } else {
            QCopChannel *channel = new QCopChannel( chan, this );
            QObject::connect
                ( channel, SIGNAL(received(QString,QByteArray)),
                  this, SLOT(received(QString,QByteArray)) );
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

void QCopAdaptor::sendMessage( const QString &msg, const QList<QVariant> &args )
{
    send( sendChannels( d->channelName ), msg, args );
}

void QCopAdaptor::send( const QStringList &channels, const QString &msg, const QList<QVariant> &args )
{
    QByteArray array;
    {
        QDataStream stream( &array, QIODevice::WriteOnly | QIODevice::Append );
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
            int *params = QSignalIntercepter::connectionTypes( name, numParams );
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
        QCopChannel::send( *iter, msg, array );
    }
}

#endif
