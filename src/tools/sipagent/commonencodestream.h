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

#ifndef COMMONENCODESTREAM_H
#define COMMONENCODESTREAM_H

#include "mediastream.h"

class CommonEncodeStream : public MediaStream
{
    Q_OBJECT
public:
    CommonEncodeStream( QObject *parent, int payloadType, int payloadSize );
    ~CommonEncodeStream();

    void setNext( MediaStream *value );

protected:
    qint64 writeData( const char *data, qint64 len );
    virtual int encode( short *in, unsigned char *out ) = 0;

private:
    short framein[256];
    unsigned char frameout[256];
    int _payloadType;
    int _payloadSize;
    int size;
};

#endif /* COMMONENCODESTREAM_H */
