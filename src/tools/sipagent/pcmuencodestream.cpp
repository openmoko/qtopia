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

#include "pcmuencodestream.h"
#include "g711.h"

PcmuEncodeStream::PcmuEncodeStream( QObject *parent )
    : CommonEncodeStream( parent, 0, 160 )
{
}

PcmuEncodeStream::~PcmuEncodeStream()
{
}

int PcmuEncodeStream::encode( short *in, unsigned char *out )
{
    int len = 160;
    while ( len-- > 0 )
        *out++ = linear2ulaw( *in++ );
    return 160;
}
