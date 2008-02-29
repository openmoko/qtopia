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

#ifndef AUDIOINPUTSTREAM_H
#define AUDIOINPUTSTREAM_H

#include "mediastream.h"

class QAudioInput;

class AudioInputStream : public MediaStream
{
    Q_OBJECT
public:
    AudioInputStream( QObject *parent );
    ~AudioInputStream();

    void start( int frequency = 8000, int channels = 1 );
    void stop();

protected:
    qint64 writeData( const char *data, qint64 len );

private slots:
    void audioReady();

private:
    QAudioInput *audio;
    char buffer[4096];
    bool dumpAudioData;
};

#endif /* AUDIOINPUTSTREAM_H */
