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

#include "sendcontactservice.h"

#include <qtopia/qcopenvelope_qws.h>

#include <qobject.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qtopia/qpeapplication.h>

SendContactService::SendContactService(QObject* parent)
    : ValueService(parent,ValueService::Set,"v-card","QCString,QString","void") // No tr
{
}

SendContactService::~SendContactService()
{
}

void SendContactService::sendParameters( QCopEnvelope &req)
{
    req << mVCard << mDescription;
}

void SendContactService::send( const QCString &vcard, const QString &description )
{
    mVCard = vcard;
    mDescription = description;

    if( serverCount() > 0 )
    {
	QDialog *dlg = new QDialog( 0, 0, TRUE, 0 );
	dlg->setCaption( tr("Send via..") );
	QVBoxLayout *l = new QVBoxLayout( dlg );
	QListBox *lb = new QListBox( dlg );
	l->addWidget( lb );
	connect( lb, SIGNAL(selected(int)), dlg, SLOT(accept()) );
	connect( lb, SIGNAL(clicked(QListBoxItem*)), dlg, SLOT(accept()) );
	connect( lb, SIGNAL(selected(QListBoxItem*)), this, 
					SLOT(serverSelected(QListBoxItem*)) );
	connect( lb, SIGNAL(clicked(QListBoxItem*)), this, 
					SLOT(serverSelected(QListBoxItem*)) );
	for( int i = 0 ; i < serverCount() ; ++i )
	{
	    ValueServiceServer src = server( i );
	    QListBoxPixmap *item = new QListBoxPixmap(lb, src.pixmap(), src.name());
	    mItemToServer.insert(item,i);
	}
	lb->sort();
	lb->setCurrentItem( 0 );
#ifndef QTOPIA_DESKTOP
	dlg->setGeometry( qApp->desktop()->geometry() );
	QPEApplication::setMenuLike( dlg, TRUE );
	QPEApplication::execDialog( dlg );
#else
	dlg->exec();
#endif
	delete dlg;
    }
}

void SendContactService::valueSupplied(QDataStream&)
{
    qWarning("SendContactService received a return value");
}

void SendContactService::serverSelected(QListBoxItem *item)
{
    request(mItemToServer[item]);
}
