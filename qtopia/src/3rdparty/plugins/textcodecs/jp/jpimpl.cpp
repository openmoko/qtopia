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
#include "jpimpl.h"

#include "qeucjpcodec.h"
#include "qjiscodec.h"
#include "qsjiscodec.h"

JpImpl::JpImpl()
{
}

JpImpl::~JpImpl()
{
}

QStringList JpImpl::names() const
{
    QStringList r;
    r.append("JIS7");
    r.append("SJIS");
    r.append("eucJP");
    return r;
}

QTextCodec *JpImpl::createForName( const QString &name )
{
    if ( name == "JIS7" )
	return new QJisCodec;
    if ( name == "SJIS" )
	return new QSjisCodec;
    if ( name == "eucJP" )
	return new QEucJpCodec;
    return 0;
}

    
QValueList<int> JpImpl::mibEnums() const
{
    QValueList<int> r;
    r.append(16);
    r.append(17);
    r.append(18);
    return r;
}

QTextCodec *JpImpl::createForMib( int mib )
{
    if ( mib == 16 )
	return new QJisCodec;
    if ( mib == 17 )
	return new QSjisCodec;
    if ( mib == 18 )
	return new QEucJpCodec;
    return 0;
}


QRESULT JpImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_QtopiaTextCodec )
	*iface = this;

    if ( *iface )
	(*iface)->addRef();
    return (*iface) ? QS_OK : QS_FALSE;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( JpImpl )
}
