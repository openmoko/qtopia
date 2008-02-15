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

#ifndef RTPSOURCESTREAM_H
#define RTPSOURCESTREAM_H

#include "mediastream.h"
#include <qlist.h>

class QUdpSocket;

class RtpSourceStream : public MediaStream
{
    Q_OBJECT
public:
    RtpSourceStream( QObject *parent );
    ~RtpSourceStream();

    int port() const;
    void addCodec( MediaStream *value );
    void setNext( MediaStream *value );

protected:
    qint64 writeData( const char *data, qint64 len );

private slots:
    void codecDestroyed();
    void readyToRead();

private:
    QList<MediaStream *> codecs;
    QUdpSocket *socket;
    char buffer[2048];
    MediaStream *cached;
    int cachedPayloadType;
};

#endif /* RTPSOURCESTREAM_H */
