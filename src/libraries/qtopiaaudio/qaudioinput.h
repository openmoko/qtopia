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

#ifndef QAUDIOINPUT_H
#define QAUDIOINPUT_H

#include <qiodevice.h>
#include <qtopiaglobal.h>

class QAudioInputPrivate;

class QTOPIAAUDIO_EXPORT QAudioInput : public QIODevice
{
    Q_OBJECT
public:
    explicit QAudioInput( const QByteArray &device, QObject *parent = 0 );
    explicit QAudioInput( QObject *parent = 0 );
    ~QAudioInput();

    int frequency() const;
    void setFrequency( int value );

    int channels() const;
    void setChannels( int value );

    int samplesPerBlock() const;
    void setSamplesPerBlock( int value );

    bool open( QIODevice::OpenMode mode );
    void close();
    bool isSequential() const;
    qint64 bytesAvailable() const;

protected:
    bool event(QEvent *ev);
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    QAudioInputPrivate *d;
};

#endif // QAUDIOINPUT_H
