/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef MEDIASTREAM_H
#define MEDIASTREAM_H

#include <qiodevice.h>

class MediaStream : public QIODevice
{
    Q_OBJECT
public:
    MediaStream( QObject *parent );
    ~MediaStream();

    MediaStream *next() const { return _next; }
    virtual void setNext( MediaStream *value );

    virtual int payloadType() const;
    virtual void setPayloadType( int type );

    virtual int payloadSize() const;
    virtual void setPayloadSize( int size );

    bool isSequential() const;
    bool open( QIODevice::OpenMode mode );
    void close();

    virtual void flush();

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeNext( const char *data, qint64 len );

private slots:
    void nextDestroyed();

private:
    MediaStream *_next;
};

#endif /* MEDIASTREAM_H */
