/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qwidget.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include "handwritingimpl.h"

#include "qimpeninput.h"

HandwritingImpl::HandwritingImpl()
    : input(0), icn(0), ref(0)
{
}

HandwritingImpl::~HandwritingImpl()
{
    delete input;
    delete icn;
}

QWidget *HandwritingImpl::inputMethod( QWidget *parent, Qt::WFlags f )
{
    if ( !input )
	input = new QIMPenInput( parent, "Handwriting", f ); // No tr
    return input;
}

void HandwritingImpl::resetState()
{
    if ( input )
	input->resetState();
}

QPixmap *HandwritingImpl::icon()
{
    if ( !icn )
	icn = new QPixmap(Resource::loadPixmap("hw/pentask")); 
    return icn;
}

QString HandwritingImpl::name()
{
    return qApp->translate( "Handwriting", "Handwriting" );
}

void HandwritingImpl::onKeyPress( QObject *receiver, const char *slot )
{
    if ( input )
	QObject::connect( input, SIGNAL(key(ushort,ushort,ushort,bool,bool)), receiver, slot );
}

#ifndef QT_NO_COMPONENT
QRESULT HandwritingImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_InputMethod )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( HandwritingImpl )
}
#endif
