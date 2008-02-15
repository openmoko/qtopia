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

#ifndef WAVOUTPUTSTREAM_H
#define WAVOUTPUTSTREAM_H

#include "mediastream.h"

class QFile;

class WavOutputStream : public MediaStream
{
    Q_OBJECT
public:
    WavOutputStream( const QString& filename, QObject *parent );
    ~WavOutputStream();

    void start( int frequency = 8000, int channels = 1 );
    void stop();

protected:
    qint64 writeData( const char *data, qint64 len );

private:
    QString filename;
    QFile *file;
    int channels;
    int frequency;
    bool writtenHeader;
    long totalSamples;
    long totalBytes;
    bool byteSwap;
    int headerLen;

    bool writeHeader();
    qint64 writeByteSwapped( const char *data, uint len );
};

#endif /* WAVOUTPUTSTREAM_H */
