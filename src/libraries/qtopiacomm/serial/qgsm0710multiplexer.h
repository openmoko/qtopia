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

#ifndef QGSM0710MULTIPLEXER_H
#define QGSM0710MULTIPLEXER_H

#include <qserialiodevicemultiplexer.h>

class QGsm0710MultiplexerPrivate;

class QTOPIACOMM_EXPORT QGsm0710Multiplexer : public QSerialIODeviceMultiplexer
{
    Q_OBJECT
    friend class QGsm0710MultiplexerPrivate;
    friend class QGsm0710MultiplexerChannel;
    friend class QGsm0710MultiplexerServer;
public:
    explicit QGsm0710Multiplexer( QSerialIODevice *device,
                                  int frameSize = 31, bool advanced = false,
                                  QObject *parent = 0 );
    ~QGsm0710Multiplexer();

    QSerialIODevice *channel( const QString& name );

    static bool cmuxChat( QSerialIODevice *device, int frameSize = 31,
                          bool advanced = false );

protected:
    virtual int channelNumber( const QString& name ) const;
    void reinit();

private slots:
    void incoming();

private:
    QGsm0710MultiplexerPrivate *d;

    QGsm0710Multiplexer( QSerialIODevice *device,
                         int frameSize, bool advanced,
                         QObject *parent, bool server );

    void terminate();
    void open( int channel );
    void close( int channel );
};

class QTOPIACOMM_EXPORT QGsm0710MultiplexerServer : public QGsm0710Multiplexer
{
    Q_OBJECT
    friend class QGsm0710Multiplexer;
public:
    explicit QGsm0710MultiplexerServer( QSerialIODevice *device,
                                        int frameSize = 31, bool advanced = false,
                                        QObject *parent = 0 );
    ~QGsm0710MultiplexerServer();

signals:
    void opened( int channel, QSerialIODevice *device );
    void closed( int channel, QSerialIODevice *device );
    void terminated();
};

#endif /* QGSM0710MULTIPLEXER_H */
