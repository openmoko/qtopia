/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef SIMPLE8IMPL_H
#define SIMPLE8IMPL_H

#include <qtopia/textcodecinterface.h>
#include <qtextcodec.h>
#include <qlist.h>

class QTOPIA_PLUGIN_EXPORT Simple8Impl : public TextCodecInterface
{
public:
    Simple8Impl();
    virtual ~Simple8Impl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual QStringList names() const;
    virtual QTextCodec *createForName( const QString &name );
    
    virtual QValueList<int> mibEnums() const;
    virtual QTextCodec *createForMib( int mib );

private:
    ulong ref;
};

#endif
