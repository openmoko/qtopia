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
#include "qobex.h"
#include "qir.h"

#include <qtranslator.h>
#define QTOPIA_INTERNAL_LANGLIST
#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>

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
  be found at <a href="http://openobex.sourceforge.net/">http://openobex.sourceforge.net/</a>
*/

// QDOC_SKIP_BEGIN

QObex::QObex()
  : ref( 0 )
{
    //we have to load languages because this is executed before the plugin 
    //loader can load translations
    QStringList langs = Global::languageList();
    for (QStringList::ConstIterator lit = langs.begin(); lit!=langs.end(); ++lit) {
	QString lang = *lit;
	QTranslator * trans = new QTranslator(qApp);
	QString tfn = QPEApplication::qpeDir()+"i18n/"+lang+"/libqobex.qm";
	if ( trans->load( tfn ))
	    qApp->installTranslator( trans );
	else
	    delete trans;
    }
    qir = new QIr();
}

/*! \internal */

QObex::~QObex()
{
    delete qir;
}

#ifndef QT_NO_COMPONENT
/*! \internal */

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

// QDOC_SKIP_END
