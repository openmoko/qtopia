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

#include "pcmudecodestream.h"
#include "g711.h"

PcmuDecodeStream::PcmuDecodeStream( QObject *parent )
    : CommonDecodeStream( parent, 0, 160 )
{
}

PcmuDecodeStream::~PcmuDecodeStream()
{
}

int PcmuDecodeStream::decode( unsigned char *in, short *out )
{
    int len = 160;
    while ( len-- > 0 )
        *out++ = ulaw2linear( *in++ );
    return 160;
}
