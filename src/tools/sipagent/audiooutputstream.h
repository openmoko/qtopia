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

#ifndef AUDIOOUTPUTSTREAM_H
#define AUDIOOUTPUTSTREAM_H

#include "mediastream.h"

class QTimer;
class QAudioOutput;

class AudioOutputStream : public MediaStream
{
    Q_OBJECT
public:
    AudioOutputStream( QObject *parent );
    ~AudioOutputStream();

    void start( int frequency = 8000, int channels = 1,
                int bitsPerSample = 16 );
    void stop();

protected:
    qint64 writeData( const char *data, qint64 len );

private slots:
    void retryOpen();

private:
    QAudioOutput *audio;
    QTimer*     timer;
    bool        isOpen;
    int         interval;
    int         retryCount;
    bool dumpAudioData;
};

#endif /* AUDIOOUTPUTSTREAM_H */
