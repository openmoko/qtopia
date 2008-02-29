/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcopchannel_qws.h"

#ifndef QT_NO_COP

#include "qwsdisplay_qws.h"
#include "qwscommand_qws_p.h"
#include "qwindowsystem_qws.h"
#include "qwindowsystem_p.h"
#include "qlist.h"
#include "qmap.h"
#include "qdatastream.h"
#include "qpointer.h"
#include "qmutex.h"

#include "qdebug.h"

typedef QMap<QString, QList<QWSClient*> > QCopServerMap;
static QCopServerMap *qcopServerMap = 0;

class QCopServerRegexp
{
public:
    QCopServerRegexp( const QString& channel, QWSClient *client );
    QCopServerRegexp( const QCopServerRegexp& other );

    QString channel;
    QWSClient *client;
    QRegExp regexp;
};

QCopServerRegexp::QCopServerRegexp( const QString& channel, QWSClient *client )
{
    this->channel = channel;
    this->client = client;
    this->regexp = QRegExp( channel, Qt::CaseSensitive, QRegExp::Wildcard );
}

QCopServerRegexp::QCopServerRegexp( const QCopServerRegexp& other )
{
    channel = other.channel;
    client = other.client;
    regexp = other.regexp;
}

typedef QList<QCopServerRegexp> QCopServerRegexpList;
static QCopServerRegexpList *qcopServerRegexpList = 0;

typedef QMap<QString, QList< QPointer<QCopChannel> > > QCopClientMap;
static QCopClientMap *qcopClientMap = 0;

Q_GLOBAL_STATIC(QMutex, qcopClientMapMutex)

// Determine if a channel name contains wildcard characters.
static bool containsWildcards( const QString& channel )
{
    return channel.contains(QLatin1Char('*'));
}

class QCopChannelPrivate
{
public:
    QString channel;
};

/*!
    \class QCopChannel
    \ingroup qws

    \brief The QCopChannel class provides communication capabilities
    between clients.

    QCOP is a many-to-many communication protocol for transferring
    messages on various channels. A channel is identified by a name,
    and anyone who wants to can listen to it. The QCOP protocol allows
    clients to communicate both within the same address space and
    between different processes, but it is currently only available
    for \l {Qtopia Core} (on X11 and Windows we are exploring the use
    of existing standards such as DCOP and COM).

    Typically, QCopChannel is either used to send messages to a
    channel using the provided static functions, or to listen to the
    traffic on a channel by deriving from the class to take advantage
    of the provided functionality for receiving messages.

    QCopChannel provides a couple of static functions which are usable
    without an object: The send() function, which sends the given
    message and data on the specified channel, and the isRegistered()
    function which queries the server for the existence of the given
    channel.

    In addition, the QCopChannel class provides the channel() function
    which returns the name of the object's channel, the virtual
    receive() function which allows subclasses to process data
    received from their channel, and the received() signal which is
    emitted with the given message and data when a QCopChannel
    subclass receives a message from its channel.

    \sa QWSClient, {Running Qtopia Core Applications}
*/

/*!
    Constructs a QCop channel with the given \a parent, and registers it
    with the server using the given \a channel name.

    \sa isRegistered(), channel()
*/

QCopChannel::QCopChannel(const QString& channel, QObject *parent) :
    QObject(parent)
{
    init(channel);
}

#ifdef QT3_SUPPORT
/*!
    Use the two argument overload, and call setObjectName() to \a name
    the instance, instead.
*/
QCopChannel::QCopChannel(const QString& channel, QObject *parent, const char *name) :
    QObject(parent)
{
    setObjectName(QString::fromAscii(name));
    init(channel);
}
#endif

void QCopChannel::init(const QString& channel)
{
    d = new QCopChannelPrivate;
    d->channel = channel;

    if (!qt_fbdpy) {
        qFatal("QCopChannel: Must construct a QApplication "
                "before QCopChannel");
        return;
    }

    {
	QMutexLocker locker(qcopClientMapMutex());

	if (!qcopClientMap)
	    qcopClientMap = new QCopClientMap;

	// do we need a new channel list ?
	QCopClientMap::Iterator it = qcopClientMap->find(channel);
	if (it != qcopClientMap->end()) {
	    it.value().append(this);
	    return;
	}

	it = qcopClientMap->insert(channel, QList< QPointer<QCopChannel> >());
	it.value().append(QPointer<QCopChannel>(this));
    }

    // inform server about this channel
    qt_fbdpy->registerChannel(channel);
}

/*!
  \internal

  Resend all channel registrations
  */
void QCopChannel::reregisterAll()
{
    if(qcopClientMap)
        for(QCopClientMap::Iterator iter = qcopClientMap->begin();
            iter != qcopClientMap->end();
            ++iter)
            qt_fbdpy->registerChannel(iter.key());
}

/*!
    Destroys the client's end of the channel and notifies the server
    that the client has closed its connection. The server will keep
    the channel open until the last registered client detaches.

    \sa QCopChannel()
*/

QCopChannel::~QCopChannel()
{
    QMutexLocker locker(qcopClientMapMutex());
    QCopClientMap::Iterator it = qcopClientMap->find(d->channel);
    Q_ASSERT(it != qcopClientMap->end());
    it.value().removeAll(this);
    // still any clients connected locally ?
    if (it.value().isEmpty()) {
        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly);
        s << d->channel;
        if (qt_fbdpy)
            send(QLatin1String(""), QLatin1String("detach()"), data);
        qcopClientMap->remove(d->channel);
    }

    delete d;
}

/*!
    Returns the name of the channel.

    \sa QCopChannel()
*/

QString QCopChannel::channel() const
{
    return d->channel;
}

/*!
    \fn void QCopChannel::receive(const QString& message, const QByteArray &data)

    This virtual function allows subclasses of QCopChannel to process
    the given \a message and \a data received from their channel. The default
    implementation emits the received() signal.

    Note that the format of the given \a data has to be well defined
    in order to extract the information it contains. In addition, it
    is recommended to use the DCOP convention. This is not a
    requirement, but you must ensure that the sender and receiver
    agree on the argument types.

    Example:

    \code
        void MyClass::receive(const QString &message, const QByteArray &data)
        {
            QDataStream in(data);
            if (message == "execute(QString,QString)") {
                QString cmd;
                QString arg;
                in >> cmd >> arg;
                ...
            } else if (message == "delete(QString)") {
                QString fileName;
                in >> fileName;
                ...
            } else {
                ...
            }
        }
    \endcode

    This example assumes that the \c message is a DCOP-style function
    signature and the \c data contains the function's arguments.

    \sa send()
 */
void QCopChannel::receive(const QString& msg, const QByteArray &data)
{
    emit received(msg, data);
}

/*!
    \fn void QCopChannel::received(const QString& message, const QByteArray &data)

    This signal is emitted with the given \a message and \a data whenever the
    receive() function gets incoming data.

    \sa receive()
*/

/*!
    Queries the server for the existence of the given \a channel. Returns true
    if the channel is registered; otherwise returns false.

    \sa channel(), QCopChannel()
*/

bool QCopChannel::isRegistered(const QString&  channel)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << channel;
    if (!send(QLatin1String(""), QLatin1String("isRegistered()"), data))
        return false;

    QWSQCopMessageEvent *e = qt_fbdpy->waitForQCopResponse();
    bool known = e->message == "known";
    delete e;
    return known;
}

/*!
    \fn bool QCopChannel::send(const QString& channel, const QString& message)
    \overload

    Sends the given \a message on the specified \a channel.  The
    message will be distributed to all clients subscribed to the
    channel.
*/

bool QCopChannel::send(const QString& channel, const QString& msg)
{
    QByteArray data;
    return send(channel, msg, data);
}

/*!
    \fn bool QCopChannel::send(const QString& channel, const QString& message,
                       const QByteArray &data)

    Sends the given \a message on the specified \a channel with the
    given \a data.  The message will be distributed to all clients
    subscribed to the channel. Returns true if the message is sent
    successfully; otherwise returns false.

    It is recommended to use the DCOP convention. This is not a
    requirement, but you must ensure that the sender and receiver
    agree on the argument types.

    Note that QDataStream provides a convenient way to fill the byte
    array with auxiliary data.

    Example:

    \code
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << QString("cat") << QString("file.txt");
        QCopChannel::send("System/Shell", "execute(QString,QString)", data);
    \endcode

    Here the channel is \c "System/Shell". The \c message is an
    arbitrary string, but in the example we've used the DCOP
    convention of passing a function signature. Such a signature is
    formatted as \c "functionname(types)" where \c types is a list of
    zero or more comma-separated type names, with no whitespace, no
    consts and no pointer or reference marks, i.e. no "*" or "&".

    \sa receive()
*/

bool QCopChannel::send(const QString& channel, const QString& msg,
                       const QByteArray &data)
{
    if (!qt_fbdpy) {
        qFatal("QCopChannel::send: Must construct a QApplication "
                "before using QCopChannel");
        return false;
    }

    qt_fbdpy->sendMessage(channel, msg, data);

    return true;
}

/*!
    \since 4.2

    Flushes any pending messages queued through QCopChannel::send() to any subscribed clients.
    Returns false if no QApplication has been constructed, otherwise returns true.
    When using QCopChannel::send(), messages are queued and actually sent when Qt re-enters the event loop.
    By using this function, an application can immediately flush these queued messages,
    and therefore reliably know that any subscribed clients will receive them.
*/
bool QCopChannel::flush()
{
    if (!qt_fbdpy) {
        qFatal("QCopChannel::flush: Must construct a QApplication "
                "before using QCopChannel");
        return false;
    }

    qt_fbdpy->flushCommands();

    return true;
}

class QWSServerSignalBridge : public QObject {
  Q_OBJECT

public:
  void emitNewChannel(const QString& channel);
  void emitRemovedChannel(const QString& channel);

  signals:
  void newChannel(const QString& channel);
  void removedChannel(const QString& channel);
};

void QWSServerSignalBridge::emitNewChannel(const QString& channel){
  emit newChannel(channel);
}

void QWSServerSignalBridge::emitRemovedChannel(const QString& channel) {
  emit removedChannel(channel);
}

/*!
    \internal
    Server side: subscribe client \a cl on channel \a ch.
*/

void QCopChannel::registerChannel(const QString& ch, QWSClient *cl)
{
    if (!qcopServerMap)
        qcopServerMap = new QCopServerMap;

    // do we need a new channel list ?
    QCopServerMap::Iterator it = qcopServerMap->find(ch);
    if (it == qcopServerMap->end())
      it = qcopServerMap->insert(ch, QList<QWSClient*>());

    // If the channel name contains wildcard characters, then we also
    // register it on the server regexp matching list.
    if (containsWildcards( ch )) {
	QCopServerRegexp item(ch, cl);
	if (!qcopServerRegexpList)
	    qcopServerRegexpList = new QCopServerRegexpList;
	qcopServerRegexpList->append( item );
    }

    // If this is the first client in the channel, announce the channel as being created.
    if (it.value().count() == 0) {
      QWSServerSignalBridge* qwsBridge = new QWSServerSignalBridge();
      connect(qwsBridge, SIGNAL(newChannel(QString)), qwsServer, SIGNAL(newChannel(QString)));
      qwsBridge->emitNewChannel(ch);
      delete qwsBridge;
    }

    it.value().append(cl);
}

/*!
    \internal
    Server side: unsubscribe \a cl from all channels.
*/

void QCopChannel::detach(QWSClient *cl)
{
    if (!qcopServerMap)
        return;

    QCopServerMap::Iterator it = qcopServerMap->begin();
    for (; it != qcopServerMap->end(); it++) {
      if (it.value().contains(cl)) {
        it.value().removeAll(cl);
        // If this was the last client in the channel, announce the channel as dead.
        if (it.value().count() == 0) {
          QWSServerSignalBridge* qwsBridge = new QWSServerSignalBridge();
          connect(qwsBridge, SIGNAL(removedChannel(QString)), qwsServer, SIGNAL(removedChannel(QString)));
          qwsBridge->emitRemovedChannel(it.key());
          delete qwsBridge;
        }
      }
    }

    if (!qcopServerRegexpList)
	return;

    QCopServerRegexpList::Iterator it2 = qcopServerRegexpList->begin();
    while(it2 != qcopServerRegexpList->end()) {
	if ((*it2).client == cl)
	    it2 = qcopServerRegexpList->erase(it2);
	else
	    ++it2;
    }
}

/*!
    \internal
    Server side: transmit the message to all clients registered to the
    specified channel.
*/

void QCopChannel::answer(QWSClient *cl, const QString& ch,
                          const QString& msg, const QByteArray &data)
{
    // internal commands
    if (ch.isEmpty()) {
        if (msg == QLatin1String("isRegistered()")) {
            QString c;
            QDataStream s(data);
            s >> c;
            bool known = qcopServerMap && qcopServerMap->contains(c)
                        && !((*qcopServerMap)[c]).isEmpty();
            QLatin1String ans = QLatin1String(known ? "known" : "unkown");
            QWSServerPrivate::sendQCopEvent(cl, QLatin1String(""),
                                            ans, data, true);
            return;
        } else if (msg == QLatin1String("detach()")) {
            QString c;
            QDataStream s(data);
            s >> c;
            Q_ASSERT(qcopServerMap);
            QCopServerMap::Iterator it = qcopServerMap->find(c);
            if (it != qcopServerMap->end()) {
                //Q_ASSERT(it.value().contains(cl));
                it.value().removeAll(cl);
                if (it.value().isEmpty()) {
                  // If this was the last client in the channel, announce the channel as dead
                  QWSServerSignalBridge* qwsBridge = new QWSServerSignalBridge();
                  connect(qwsBridge, SIGNAL(removedChannel(QString)), qwsServer, SIGNAL(removedChannel(QString)));
                  qwsBridge->emitRemovedChannel(it.key());
                  delete qwsBridge;
                  qcopServerMap->erase(it);
                }
            }
	    if (qcopServerRegexpList && containsWildcards(c)) {
		// Remove references to a wildcarded channel.
		QCopServerRegexpList::Iterator it
		    = qcopServerRegexpList->begin();
		while(it != qcopServerRegexpList->end()) {
		    if ((*it).client == cl && (*it).channel == c)
			it = qcopServerRegexpList->erase(it);
		    else
			++it;
		}
	    }
            return;
        }
        qWarning("QCopChannel: unknown internal command %s", qPrintable(msg));
        QWSServerPrivate::sendQCopEvent(cl, QLatin1String(""),
                                        QLatin1String("bad"), data);
        return;
    }

    if (qcopServerMap) {
        QList<QWSClient*> clist = qcopServerMap->value(ch);
        for (int i=0; i < clist.size(); ++i) {
            QWSClient *c = clist.at(i);
            QWSServerPrivate::sendQCopEvent(c, ch, msg, data);
        }
    }

    if(qcopServerRegexpList && !containsWildcards(ch)) {
	// Search for wildcard matches and forward the message on.
	QCopServerRegexpList::ConstIterator it = qcopServerRegexpList->constBegin();
	for (; it != qcopServerRegexpList->constEnd(); ++it) {
	    if ((*it).regexp.exactMatch(ch)) {
		QByteArray newData;
		{
		    QDataStream stream
			(&newData, QIODevice::WriteOnly | QIODevice::Append);
		    stream << ch;
		    stream << msg;
		    stream << data;
		    // Stream is flushed and closed at this point.
		}
		QWSServerPrivate::sendQCopEvent
		    ((*it).client, (*it).channel,
		     QLatin1String("forwardedMessage(QString,QString,QByteArray)"),
		     newData);
	    }
	}
    }
}

/*!
    \internal
    Client side: distribute received event to the QCop instance managing the
    channel.
*/
void QCopChannel::sendLocally(const QString& ch, const QString& msg,
                                const QByteArray &data)
{
    Q_ASSERT(qcopClientMap);

    // filter out internal events
    if (ch.isEmpty())
        return;

    // feed local clients with received data
    QList< QPointer<QCopChannel> > clients;
    {
	QMutexLocker locker(qcopClientMapMutex());
	clients = (*qcopClientMap)[ch];
    }
    for (int i = 0; i < clients.size(); ++i) {
	QCopChannel *channel = (QCopChannel *)clients.at(i);
	if ( channel )
	    channel->receive(msg, data);
    }
}
#include "qcopchannel_qws.moc"

#endif
