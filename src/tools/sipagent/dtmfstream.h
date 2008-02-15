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

#ifndef DTMFSTREAM_H
#define DTMFSTREAM_H

#include "mediastream.h"

class DtmfStream : public MediaStream
{
    Q_OBJECT
public:
    DtmfStream( QObject *parent );
    ~DtmfStream();

public slots:
    void dtmf( const QString& tones );

protected:
    qint64 writeData( const char *data, qint64 len );

private:
    QString pendingTones;
    bool generatingTones;
    static short const sine[8000];
    int lower, higher, offset;
    int samplesToGenerate, gapSamples;

    bool getToneFrequencies( int& lower, int& higher );
    void nextTone();
};

#endif /* DTMFSTREAM_H */
