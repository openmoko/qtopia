/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qbuffer.h"
#include "private/qiodevice_p.h"

/** QBufferPrivate **/
class QBufferPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QBuffer)

public:
    QBufferPrivate()
        :
#ifndef QT_NO_QOBJECT
        signalsEmitted(false), writtenSinceLastEmit(0),
#endif
        buf(0)  { }
    ~QBufferPrivate() { }

#ifndef QT_NO_QOBJECT
    // private slots
    void _q_emitSignals();

    bool signalsEmitted;
    qint64 writtenSinceLastEmit;
#endif

    QByteArray *buf;
    int ioIndex;

    QByteArray defaultBuf;
};

#ifndef QT_NO_QOBJECT
void QBufferPrivate::_q_emitSignals()
{
    Q_Q(QBuffer);
    emit q->bytesWritten(writtenSinceLastEmit);
    writtenSinceLastEmit = 0;
    emit q->readyRead();
    signalsEmitted = false;
}
#endif

/*!
    \class QBuffer
    \reentrant
    \brief The QBuffer class provides a QIODevice interface for a QByteArray.

    \ingroup io

    QBuffer allows you to access a QByteArray using the QIODevice
    interface. The QByteArray is treated just as a standard random-accessed
    file. Example:

    \quotefromfile snippets/buffer/buffer.cpp
    \skipto main_snippet
    \skipto QBuffer buffer
    \printto /^\}/

    By default, an internal QByteArray buffer is created for you when
    you create a QBuffer. You can access this buffer directly by
    calling buffer(). You can also use QBuffer with an existing
    QByteArray by calling setBuffer(), or by passing your array to
    QBuffer's constructor.

    Call open() to open the buffer. Then call write() or
    putChar() to write to the buffer, and read(), readLine(),
    readAll(), or getChar() to read from it. size() returns the
    current size of the buffer, and you can seek to arbitrary
    positions in the buffer by calling seek(). When you are done with
    accessing the buffer, call close().

    The following code snippet shows how to write data to a
    QByteArray using QDataStream and QBuffer:

    \skipto write_datastream_snippet
    \skipto QByteArray
    \printto /^\}/

    Effectively, we convert the application's QPalette into a byte
    array. Here's how to read the data from the QByteArray:

    \skipto read_datastream_snippet
    \skipto QPalette
    \printto /^\}/

    QTextStream and QDataStream also provide convenience constructors
    that take a QByteArray and that create a QBuffer behind the
    scenes.

    QBuffer emits readyRead() when new data has arrived in the
    buffer. By connecting to this signal, you can use QBuffer to
    store temporary data before processing it. For example, you can
    pass the buffer to QFtp when downloading a file from an FTP
    server. Whenever a new payload of data has been downloaded,
    readyRead() is emitted, and you can process the data that just
    arrived. QBuffer also emits bytesWritten() every time new data
    has been written to the buffer.

    \sa QFile, QDataStream, QTextStream, QByteArray
*/

#ifdef QT_NO_QOBJECT
QBuffer::QBuffer()
    : QIODevice(*new QBufferPrivate)
{
    Q_D(QBuffer);
    d->buf = &d->defaultBuf;
    d->ioIndex = 0;
}
QBuffer::QBuffer(QByteArray *buf)
    : QIODevice(*new QBufferPrivate)
{
    Q_D(QBuffer);
    d->buf = buf ? buf : &d->defaultBuf;
    d->ioIndex = 0;
    d->defaultBuf.clear();
}
#else
/*!
    Constructs an empty buffer with the given \a parent. You can call
    setData() to fill the buffer with data, or you can open it in
    write mode and use write().

    \sa open()
*/
QBuffer::QBuffer(QObject *parent)
    : QIODevice(*new QBufferPrivate, parent)
{
    Q_D(QBuffer);
    d->buf = &d->defaultBuf;
    d->ioIndex = 0;
}

/*!
    Constructs a QBuffer that uses the QByteArray pointed to by \a
    byteArray as its internal buffer, and with the given \a parent.
    The caller is responsible for ensuring that \a byteArray remains
    valid until the QBuffer is destroyed, or until setBuffer() is
    called to change the buffer. QBuffer doesn't take ownership of
    the QByteArray.

    If you open the buffer in write-only mode or read-write mode and
    write something into the QBuffer, \a byteArray will be modified.

    Example:

    \quotefromfile snippets/buffer/buffer.cpp
    \skipto bytearray_ptr_ctor_snippet
    \skipto QByteArray
    \printto /^\}/

    \sa open(), setBuffer(), setData()
*/
QBuffer::QBuffer(QByteArray *byteArray, QObject *parent)
    : QIODevice(*new QBufferPrivate, parent)
{
    Q_D(QBuffer);
    d->buf = byteArray ? byteArray : &d->defaultBuf;
    d->defaultBuf.clear();
    d->ioIndex = 0;
}
#endif

/*!
    Destroys the buffer.
*/

QBuffer::~QBuffer()
{
}

/*!
    Makes QBuffer uses the QByteArray pointed to by \a
    byteArray as its internal buffer. The caller is responsible for
    ensuring that \a byteArray remains valid until the QBuffer is
    destroyed, or until setBuffer() is called to change the buffer.
    QBuffer doesn't take ownership of the QByteArray.

    Does nothing if isOpen() is true.

    If you open the buffer in write-only mode or read-write mode and
    write something into the QBuffer, \a byteArray will be modified.

    Example:

    \quotefromfile snippets/buffer/buffer.cpp
    \skipto setBuffer_snippet
    \skipto QByteArray
    \printto /^\}/

    If \a byteArray is 0, the buffer creates its own internal
    QByteArray to work on. This byte array is initially empty.

    \sa buffer(), setData(), open()
*/

void QBuffer::setBuffer(QByteArray *byteArray)
{
    Q_D(QBuffer);
    if (isOpen()) {
        qWarning("QBuffer::setBuffer: Buffer is open");
        return;
    }
    if (byteArray) {
        d->buf = byteArray;
    } else {
        d->buf = &d->defaultBuf;
    }
    d->defaultBuf.clear();
    d->ioIndex = 0;
}

/*!
    Returns a reference to the QBuffer's internal buffer. You can use
    it to modify the QByteArray behind the QBuffer's back.

    \sa setBuffer(), data()
*/

QByteArray &QBuffer::buffer()
{
    Q_D(QBuffer);
    return *d->buf;
}

/*!
    \overload

    This is the same as data().
*/

const QByteArray &QBuffer::buffer() const
{
    Q_D(const QBuffer);
    return *d->buf;
}


/*!
    Returns the data contained in the buffer.

    This is the same as buffer().

    \sa setData(), setBuffer()
*/

const QByteArray &QBuffer::data() const
{
    Q_D(const QBuffer);
    return *d->buf;
}

/*!
    Sets the contents of the internal buffer to be \a data. This is
    the same as assigning \a data to buffer().

    Does nothing if isOpen() is true.

    \sa setBuffer()
*/
void QBuffer::setData(const QByteArray &data)
{
    Q_D(QBuffer);
    if (isOpen()) {
        qWarning("QBuffer::setData: Buffer is open");
        return;
    }
    *d->buf = data;
    d->ioIndex = 0;
}

/*!
    \fn void QBuffer::setData(const char *data, int size)

    \overload

    Sets the contents of the internal buffer to be the first \a size
    bytes of \a data.
*/

/*!
   \reimp
*/
bool QBuffer::open(OpenMode flags)
{
    Q_D(QBuffer);

    if ((flags & Append) == Append)
        flags |= WriteOnly;
    setOpenMode(flags);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }

    if ((flags & QIODevice::Truncate) == QIODevice::Truncate) {
        d->buf->resize(0);
    }
    if ((flags & QIODevice::Append) == QIODevice::Append) // append to end of buffer
        seek(d->buf->size());
    else
        seek(0);

    return true;
}

/*!
    \reimp
*/
void QBuffer::close()
{
    QIODevice::close();
}

/*!
    \reimp
*/
qint64 QBuffer::pos() const
{
    return QIODevice::pos();
}

/*!
    \reimp
*/
qint64 QBuffer::size() const
{
    Q_D(const QBuffer);
    return qint64(d->buf->size());
}

/*!
    \reimp
*/
bool QBuffer::seek(qint64 pos)
{
    Q_D(QBuffer);
    if (pos < 0 || pos >= d->buf->size() + 1) {
        qWarning("QBuffer::seek: Invalid pos: %d", int(pos));
        return false;
    }
    d->ioIndex = int(pos);
    return QIODevice::seek(pos);
}

/*!
    \reimp
*/
bool QBuffer::atEnd() const
{
    return QIODevice::atEnd();
}

/*!
   \reimp
*/
bool QBuffer::canReadLine() const
{
    Q_D(const QBuffer);
    if (!isOpen())
        return false;

    return d->buf->indexOf('\n', int(pos())) != -1 || QIODevice::canReadLine();
}

/*!
    \reimp
*/
qint64 QBuffer::readData(char *data, qint64 len)
{
    Q_D(QBuffer);
    if ((len = qMin(len, qint64(d->buf->size()) - d->ioIndex)) <= 0)
        return qint64(0);
    memcpy(data, d->buf->constData() + d->ioIndex, len);
    d->ioIndex += int(len);
    return len;
}

/*!
    \reimp
*/
qint64 QBuffer::writeData(const char *data, qint64 len)
{
    Q_D(QBuffer);
    int extraBytes = d->ioIndex + len - d->buf->size();
    if (extraBytes > 0) { // overflow
        int newSize = d->buf->size() + extraBytes;
        d->buf->resize(newSize);
        if (d->buf->size() != newSize) { // could not resize
            qWarning("QBuffer::writeData: Memory allocation error");
            return -1;
        }
    }

    memcpy(d->buf->data() + d->ioIndex, (uchar *)data, int(len));
    d->ioIndex += int(len);

#ifndef QT_NO_QOBJECT
    d->writtenSinceLastEmit += len;
    if (!d->signalsEmitted) {
        d->signalsEmitted = true;
        QMetaObject::invokeMethod(this, "_q_emitSignals", Qt::QueuedConnection);
    }
#endif
    return len;
}

#ifndef QT_NO_QOBJECT
# include "moc_qbuffer.cpp"
#endif
