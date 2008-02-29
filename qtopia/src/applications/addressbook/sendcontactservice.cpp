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
#include <qfile.h>

SendContactService::SendContactService(QObject* parent)
    : ValueService(parent,ValueService::Set,"v-card","","void") // No tr
{
}

SendContactService::~SendContactService()
{
}

void SendContactService::sendParameters( QCopEnvelope &req)
{
    if( mSelectedParams == 3 )
        req << mContactList[0].uid();
    else
        req << makeVCardFile() << mDescription;
}

void SendContactService::send( const QValueList<PimContact>& contactList, const QString &description, int field )
{
    //
    //  Construct and display a list of services to potentially send to. This code assumes that
    //  v-card sending services either accept (QString,QCString,QString,QCString) for sending
    //  a file containing one or more full v-cards, or just (QUuid) for sending one contact's
    //  v-card.
    //
    
    mField = field;
    mContactList = contactList;
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
            //  Special case: v-card sending services that only accept a single
            //  QUuid can only send one contact.
            if( contactList.count() <= 1 || server(i).paramCount() > 3 )
            {
                ValueServiceServer src = server( i );
                QListBoxPixmap *item = new QListBoxPixmap(lb, src.pixmap(), src.name());
                mItemToServer.insert(item,i);
            }
	}
	lb->sort();
	lb->setCurrentItem( 0 );
        mRequestSent = FALSE;
#ifndef QTOPIA_DESKTOP
	dlg->setGeometry( qApp->desktop()->geometry() );
	QPEApplication::setMenuLike( dlg, TRUE );
	if( QPEApplication::execDialog( dlg ) == QDialog::Accepted )
#else
	if( dlg->exec() == QDialog::Accepted )
#endif
        {
            if( lb->currentItem() >= 0 && !mRequestSent )
                serverSelected( lb->item( lb->currentItem() ) );
        }
	delete dlg;
    }
}

void SendContactService::valueSupplied(QDataStream&)
{
    qWarning("SendContactService received a return value");
}

void SendContactService::serverSelected(QListBoxItem *item)
{
    mRequestSent = TRUE;
    mSelectedParams = server(mItemToServer[item]).paramCount();
    if( mSelectedParams == 3 )
        changeArgs( "QUuid" );
    else
        changeArgs( "QCString,QString" );
        
    request( mItemToServer[item] );
}

QCString SendContactService::makeVCardFile()
{
    QString filename;
    for( int i = 0 ; i < (int)mDescription.length() ; ++i )
	if( mDescription[i].isLetterOrNumber() || mDescription[i] == '-' )
	    filename += mDescription[i];
	else if( mDescription[i].isSpace() )
	    filename += '-';
    if( filename.isEmpty() )
	filename = "contact";
    filename = QString("%1%2-vcard.vcf").arg(Global::tempDir()).arg( filename );
    
    if(mField == -1)
        PimContact::writeVCard( filename.utf8(), mContactList );
    else
    {
        //  If a field is specified, we are sending only one contact.
        QFile vCardFile(filename.utf8());
        QString vcard;
        PimContact cnt = mContactList[0];

        vCardFile.open(IO_WriteOnly);
                
        vcard.sprintf("BEGIN:VCARD\n"
                "VERSION:2.1\n"
                "FN:%s\n"
                "TEL;HOME:%s\n"
                "END:VCARD",
                (const char*)cnt.fullName(),
                (const char*)cnt.field(mField));
        vCardFile.writeBlock((const char*)vcard, vcard.length());
        vCardFile.close();
    }
    
    return QCString((const char*)filename);
}


















