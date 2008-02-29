/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "menumodel.h"

#include <servicerequest.h>

SimpleMenuModel::~SimpleMenuModel()
{
    foreach( TextRequestPair item, m_items ) {
        delete item.request;
    }
}

void SimpleMenuModel::addItem( const QString& text, const ServiceRequest& request )
{
    TextRequestPair pair;
    pair.text = text;
    pair.request = request.clone();

    m_items.append( pair );
}

void SimpleMenuModel::addItem( const QString& text, QObject* receiver, const char* member )
{
    TextRequestPair pair;
    pair.text = text;
    pair.request = new TriggerSlotRequest( receiver, member );

    m_items.append( pair );
}

void SimpleMenuModel::addItem( const QString& text, MenuModel* model )
{
    TextRequestPair pair;
    pair.text = text;

    PushMenuRequest pushmenu( model );
    PushTitleRequest pushtitle( text );
    pair.request = new CompoundRequest( QList<ServiceRequest*>() << &pushmenu << &pushtitle );

    m_items.append( pair );
}

ServiceRequest* SimpleMenuModel::action( const QModelIndex& index, ActionContext context ) const
{
    if( context == Select ) {
        TextRequestPair item = m_items[index.row()];

        return item.request->clone();
    }

    return 0;
}

QVariant SimpleMenuModel::data( const QModelIndex& index, int role ) const
{
    if( role == Qt::DisplayRole ) {
        TextRequestPair item = m_items[index.row()];

        return item.text;
    }

    return QVariant();
}
