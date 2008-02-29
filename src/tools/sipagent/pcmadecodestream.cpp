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

#include "pcmadecodestream.h"
#include "g711.h"

PcmaDecodeStream::PcmaDecodeStream( QObject *parent )
    : CommonDecodeStream( parent, 8, 160 )
{
}

PcmaDecodeStream::~PcmaDecodeStream()
{
}

int PcmaDecodeStream::decode( unsigned char *in, short *out )
{
    int len = 160;
    while ( len-- > 0 )
        *out++ = alaw2linear( *in++ );
    return 160;
}
