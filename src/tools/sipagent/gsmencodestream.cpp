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

#include "gsmencodestream.h"
extern "C" {
#include "gsm/gsm.h"
};

GsmEncodeStream::GsmEncodeStream( QObject *parent )
    : CommonEncodeStream( parent, 3, 160 )
{
    state = gsm_create();
}

GsmEncodeStream::~GsmEncodeStream()
{
    gsm_destroy( state );
}

int GsmEncodeStream::encode( short *in, unsigned char *out )
{
    gsm_encode( state, in, out );
    return 33;
}
