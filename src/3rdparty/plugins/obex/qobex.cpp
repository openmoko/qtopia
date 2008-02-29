/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qobex.h"
#include "qir.h"

/*!
  \class QObex qobex.h
  \brief The QObex class provides an interface to OpenOBEX.

  \legalese
  QObex links with OpenOBEX, which is an open source implementation
  of the Object Exchange (OBEX) protocol. This protocol is essentially
  a typed file-exchange protocol. OpenOBEX is distributed under the terms
  of the GNU Lesser General Public License (LGPL). Primary
  copyright holders of OpenOBEX include Dag Brattli and
  Pontus Fuchs. QObex is used by Qtopia for infra-red communications.

  The version of OpenOBEX included here includes some functions
  of GLIB, which is distributed under the terms of the GNU Library General Public
  License (LGPL). The primary copyright holders of GLIB are Peter Mattis,
  Spencer Kimball and Josh MacDonald. More information about OpenOBEX can
  be found at http://openobex.sourceforge.net/
*/

QObex::QObex()
  : ref( 0 )
{
    qir = new QIr();
}

QObex::~QObex()
{
    delete qir;
}

#ifndef QT_NO_COMPONENT
QRESULT QObex::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_ObexInterface )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( QObex )
}
#endif
