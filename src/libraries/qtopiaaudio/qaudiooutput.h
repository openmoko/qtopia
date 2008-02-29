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

#ifndef QAUDIOOUTPUT_H
#define QAUDIOOUTPUT_H

#include <qiodevice.h>

#include <qtopiaglobal.h>

class QAudioOutputPrivate;

class QTOPIAAUDIO_EXPORT QAudioOutput : public QIODevice
{
    Q_OBJECT
public:
    explicit QAudioOutput( QObject *parent = 0 );
    explicit QAudioOutput( const QByteArray &device, QObject *parent = 0 );
    ~QAudioOutput();

    int frequency() const;
    void setFrequency( int value );

    int channels() const;
    void setChannels( int value );

    int bitsPerSample() const;
    void setBitsPerSample( int value );

    bool open( QIODevice::OpenMode mode );
    void close();
    bool isSequential() const;

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void deviceReady( int id );
    void deviceError( int id );

private:
    QAudioOutputPrivate *d;
};

#endif // QAUDIOOUTPUT_H
