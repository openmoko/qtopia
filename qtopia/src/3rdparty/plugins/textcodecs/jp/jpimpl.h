/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef JPIMPL_H
#define JPIMPL_H

#include <qtopia/textcodecinterface.h>
#include <qtextcodec.h>
#include <qlist.h>

class JpImpl : public TextCodecInterface
{
public:
    JpImpl();
    virtual ~JpImpl();

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
