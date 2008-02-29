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

#ifndef RTPSINKSTREAM_H
#define RTPSINKSTREAM_H

#include "mediastream.h"
#include <qhostaddress.h>
#include <qhostinfo.h>
#include <qbytearray.h>

class QUdpSocket;

class RtpSinkStream : public MediaStream
{
    Q_OBJECT
public:
    RtpSinkStream( QObject *parent );
    ~RtpSinkStream();

    void setDestination( const QString& hostName, int port );

    int payloadType() const;
    void setPayloadType( int type );

    int payloadSize() const;
    void setPayloadSize( int size );

    int synchronizationSource() const;
    void setSynchronizationSource( int source );

protected:
    qint64 writeData( const char *data, qint64 len );

signals:
    void readyToWrite();
    void lookupFailed();

private slots:
    void lookupDone( const QHostInfo& info );

private:
    int _payloadType;
    int _payloadSize;
    QUdpSocket *socket;
    QHostAddress address;
    quint16 port;
    int sequence;
    int timestamp;
    int source;
    QByteArray buffer;

    void makeSocket();
};

#endif /* RTPSINKSTREAM_H */
