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

#ifndef GSMDECODESTREAM_H
#define GSMDECODESTREAM_H

#include "commondecodestream.h"

class GsmDecodeStream : public CommonDecodeStream
{
    Q_OBJECT
public:
    GsmDecodeStream( QObject *parent );
    ~GsmDecodeStream();

protected:
    int decode( unsigned char *in, short *out );

private:
    struct gsm_state *state;
};

#endif /* GSMDECODESTREAM_H */
