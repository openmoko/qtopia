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

#ifndef COMMONDECODESTREAM_H
#define COMMONDECODESTREAM_H

#include "mediastream.h"

class CommonDecodeStream : public MediaStream
{
    Q_OBJECT
public:
    CommonDecodeStream( QObject *parent, int payloadType, int packetSize );
    ~CommonDecodeStream();

    int payloadType() const;

protected:
    qint64 writeData( const char *data, qint64 len );
    virtual int decode( unsigned char *in, short *out ) = 0;

private:
    short frameout[256];
    int _payloadType;
    int _packetSize;
};

#endif /* COMMONDECODESTREAM_H */
