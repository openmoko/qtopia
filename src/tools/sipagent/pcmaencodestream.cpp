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

#include "pcmaencodestream.h"
#include "g711.h"

PcmaEncodeStream::PcmaEncodeStream( QObject *parent )
    : CommonEncodeStream( parent, 8, 160 )
{
}

PcmaEncodeStream::~PcmaEncodeStream()
{
}

int PcmaEncodeStream::encode( short *in, unsigned char *out )
{
    int len = 160;
    while ( len-- > 0 )
        *out++ = linear2alaw( *in++ );
    return 160;
}
