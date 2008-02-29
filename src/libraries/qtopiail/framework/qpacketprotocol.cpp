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

#include "qpacketprotocol.h"
#include <QBuffer>

#define MAX_PACKET_SIZE 0x7FFFFFFF

/*!
  \class QPacketProtocol
  \brief The QPacketProtocol class encapsulates communicating discrete packets
  across fragmented IO channels, such as TCP sockets.

  As transmission boundaries are not respected, sending packets over protocols
  like TCP frequently involves "stitching" them back together at the receiver.
  QPacketProtocol makes this easier by performing this task for you.  Packet
  data sent using QPacketProtocol is prepended with a 4-byte size header
  allowing the receiving QPacketProtocol to buffer the packet internally until
  it has all been received.  QPacketProtocol does not perform any sanity
  checking on the size or on the data, so this class should only be used in
  prototyping or trusted situations where DOS attacks are unlikely.

  QPacketProtocol does not perform any communications itself.  Instead it can
  operate on any QIODevice that supports the QIODevice::readyRead() signal.  A
  logical "packet" is encapsulated by the companion QPacket class.  The
  following example shows two ways to send data using QPacketProtocol.  The
  transmitted data is equivalent in both.

  \code
  QTcpSocket socket;
  // ... connect socket ...

  QPacketProtocol protocol(&socket);

  // Send packet the quick way
  protocol.send() << 12 << "Hello world";

  // Send packet the longer way
  QPacket packet;
  packet << 12 << "Hello world";
  protocol.send(packet);
  \endcode

  Likewise, the following shows how to read data from QPacketProtocol, assuming
  that the QPacketProtocol::readyRead() signal has been emitted.

  \code
  // ... QPacketProtocol::readyRead() is emitted ...

  int a;
  QByteArray b;

  // Receive packet the quick way
  protocol.read() >> a >> b;

  // Receive packet the longer way
  QPacket packet = protocol.read();
  p >> a >> b;
  \endcode
*/

class QPacketProtocolPrivate : public QObject
{
Q_OBJECT
public:
    QPacketProtocolPrivate(QPacketProtocol * parent, QIODevice * _dev)
    : QObject(parent), inProgressSize(-1), maxPacketSize(MAX_PACKET_SIZE),
      dev(_dev)
    {
        Q_ASSERT(4 == sizeof(qint32));

        QObject::connect(this, SIGNAL(readyRead()),
                         parent, SIGNAL(readyRead()));
        QObject::connect(this, SIGNAL(packetWritten()),
                         parent, SIGNAL(packetWritten()));
        QObject::connect(this, SIGNAL(invalidPacket()),
                         parent, SIGNAL(invalidPacket()));
        QObject::connect(dev, SIGNAL(readyRead()),
                         this, SLOT(readyToRead()));
        QObject::connect(dev, SIGNAL(aboutToClose()),
                         this, SLOT(aboutToClose()));
        QObject::connect(dev, SIGNAL(bytesWritten(qint64)),
                         this, SLOT(bytesWritten(qint64)));
    }

signals:
    void readyRead();
    void packetWritten();
    void invalidPacket();

public slots:
    void aboutToClose()
    {
        inProgress.clear();
        sendingPackets.clear();
        inProgressSize = -1;
    }

    void bytesWritten(qint64 bytes)
    {
        Q_ASSERT(!sendingPackets.isEmpty());

        while(bytes) {
            if(sendingPackets.at(0) > bytes) {
                sendingPackets[0] -= bytes;
                bytes = 0;
            } else {
                bytes -= sendingPackets.at(0);
                sendingPackets.removeFirst();
                emit packetWritten();
            }
        }
    }

    void readyToRead()
    {
        if(-1 == inProgressSize) {
            // We need a size header of sizeof(qint32)
            if(sizeof(qint32) > dev->bytesAvailable())
                return;

            // Read size header
            int read = dev->read((char *)&inProgressSize, sizeof(qint32));
            Q_ASSERT(read == sizeof(qint32));
            Q_UNUSED(read);

            // Check sizing constraints
            if(inProgressSize > maxPacketSize) {
                QObject::disconnect(dev, SIGNAL(readyRead()),
                                    this, SLOT(readyToRead()));
                QObject::disconnect(dev, SIGNAL(aboutToClose()),
                                    this, SLOT(aboutToClose()));
                QObject::disconnect(dev, SIGNAL(bytesWritten(qint64)),
                                    this, SLOT(bytesWritten(qint64)));
                dev = 0;
                emit invalidPacket();
                return;
            }

            inProgressSize -= sizeof(qint32);

            // Need to get trailing data
            readyToRead();
        } else {
            inProgress.append(dev->read(inProgressSize - inProgress.size()));

            if(inProgressSize == inProgress.size()) {
                // Packet has arrived!
                packets.append(inProgress);
                inProgressSize = -1;
                inProgress.clear();

                emit readyRead();

                // Need to get trailing data
                readyToRead();
            }
        }
    }

public:
    QList<qint64> sendingPackets;
    QList<QByteArray> packets;
    QByteArray inProgress;
    qint32 inProgressSize;
    qint32 maxPacketSize;
    QIODevice * dev;
};

/*!
  Construct a QPacketProtocol instance that works on \a dev with the
  specified \a parent.
 */
QPacketProtocol::QPacketProtocol(QIODevice * dev, QObject * parent)
: QObject(parent), d(new QPacketProtocolPrivate(this, dev))
{
    Q_ASSERT(dev);
}

/*!
  Destroys the QPacketProtocol instance.
 */
QPacketProtocol::~QPacketProtocol()
{
}

/*!
  Returns the maximum packet size allowed.  By default this is
  2,147,483,647 bytes.  If a packet claiming to be larger than this is received,
  the QPacketProtocol::invalidPacket() signal is emitted.

  \sa QPacketProtocol::setMaximumPacketSize()
 */
qint32 QPacketProtocol::maximumPacketSize() const
{
    return d->maxPacketSize;
}

/*!
  Sets the maximum allowable packet size to \a max.

  \sa QPacketProtocol::maximumPacketSize()
 */
qint32 QPacketProtocol::setMaximumPacketSize(qint32 max)
{
    if(max > (signed)sizeof(qint32))
        d->maxPacketSize = max;
    return d->maxPacketSize;
}

/*!
  Returns a streamable object that is transmitted on destruction.  For example

  \code
  protocol.send() << 10 << "Hello world";
  \endcode
 */
QPacketAutoSend QPacketProtocol::send()
{
    return QPacketAutoSend(this);
}

/*!
  \fn void QPacketProtocol::send(const QPacket & packet)
  Transmit the \a packet.
 */
void QPacketProtocol::send(const QPacket & p)
{
    if(p.b.isEmpty())
        return; // We don't send empty packets

    qint64 sendSize = p.b.size() + sizeof(qint32);

    d->sendingPackets.append(sendSize);
    qint64 writeBytes = d->dev->write((char *)&sendSize, sizeof(qint32));
    Q_ASSERT(writeBytes == sizeof(qint32));
    writeBytes = d->dev->write(p.b);
    Q_ASSERT(writeBytes == p.b.size());
}

/*!
  Returns the number of received packets yet to be read.
  */
qint64 QPacketProtocol::packetsAvailable() const
{
    return d->packets.count();
}

/*!
  Discard any unread packets.
  */
void QPacketProtocol::clear()
{
    d->packets.clear();
}

/*!
  Return the next unread packet, or an invalid QPacket instance if no packets
  are available.  This method does NOT block.
  */
QPacket QPacketProtocol::read()
{
    if(0 == d->packets.count())
        return QPacket();

    QPacket rv(d->packets.at(0));
    d->packets.removeFirst();
    return rv;
}

/*!
  Return the QIODevice passed to the QPacketProtocol constructor.
*/
QIODevice * QPacketProtocol::device()
{
    return d->dev;
}

/*!
  \fn void QPacketProtocol::readyRead()

  Emitted whenever a new packet is received.  Applications may use
  QPacketProtocol::read() to retrieve this packet.
 */

/*!
  \fn void QPacketProtocol::invalidPacket()

  A packet larger than the maximum allowable packet size was received.  The
  packet will be discarded and, as it indicates corruption in the protocol, no
  further packets will be received.
 */

/*!
  \fn void QPacketProtocol::packetWritten()

  Emitted each time a packet is completing written to the device.  This signal
  may be used for communications flow control.
 */

/*!
  \class QPacket
  \brief The QPacket class encapsulates an unfragmentable packet of data to be
  transmitted by QPacketProtocol.

  QPacket provides a QDataStream interface to an unfragmentable packet.
  Applications should construct a QPacket, propagate it with data and then
  transmit it over a QPacketProtocol instance.  For example:
  \code
  QPacketProtocol protocol(...);

  QPacket myPacket;
  myPacket << 10 << "Hello world!";
  protocol.send(myPacket);
  \endcode

  As long as both ends of the connection are using the QPacketProtocol class,
  the data within this packet will be delivered unfragmented at the other end,
  ready for extraction.

  \code
  int a;
  QByteArray b;

  myPacket >> a >> b;
  \endcode

  Only packets returned by QPacketProtocol may be read from.
 */

/*!
  Constructs an empty write-only packet.
  */
QPacket::QPacket()
: QDataStream(), buf(0)
{
    buf = new QBuffer(&b);
    buf->open(QIODevice::WriteOnly);
    setDevice(buf);
}

/*!
  Destroys the QPacket instance.
  */
QPacket::~QPacket()
{
    if(buf) {
        delete buf;
        buf = 0;
    }
}

/*!
  Creates a copy of \a other.  The initial stream positions are shared, but the
  two packets are otherwise independant.
 */
QPacket::QPacket(const QPacket & other)
: QDataStream(), b(other.b), buf(0)
{
    buf = new QBuffer(&b);
    buf->open(other.buf->openMode());
    setDevice(buf);
}

/*!
  \internal
  */
QPacket::QPacket(const QByteArray & ba)
: QDataStream(), b(ba), buf(0)
{
    buf = new QBuffer(&b);
    buf->open(QIODevice::ReadOnly);
    setDevice(buf);
}

/*!
  Returns true if this packet is empty - that is, contains no data.
  */
bool QPacket::isEmpty() const
{
    return b.isEmpty();
}

/*!
  Clears data in the packet.  This is useful for reusing one writable packet.
  For example
  \code
  QPacketProtocol protocol(...);

  QPacket packet;

  packet << 10 << "Hello world!";
  protocol.send(packet);

  packet.clear();
  packet << 11 << "Goodbyte world!";
  protocol.send(packet);
  \endcode
 */
void QPacket::clear()
{
    QBuffer::OpenMode oldMode = buf->openMode();
    buf->close();
    b.clear();
    buf->setBuffer(&b); // reset QBuffer internals with new size of b.
    buf->open(oldMode);
}

/*!
  \class QPacketAutoSend
  \internal
  */
QPacketAutoSend::QPacketAutoSend(QPacketProtocol * _p)
: QPacket(), p(_p)
{
}

QPacketAutoSend::~QPacketAutoSend()
{
    if(!b.isEmpty())
        p->send(*this);
}

#include "qpacketprotocol.moc"

