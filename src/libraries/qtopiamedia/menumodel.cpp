/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "menumodel_p.h"

#include <private/servicerequest_p.h>

/*!
    \class PlaylistModel
    \internal
*/

/*!
    \fn PlaylistModel::~PlaylistModel()
    \internal
*/

/*!
    \fn Playlist* PlaylistModel::playlist() const = 0;
    \internal
*/

/*!
    \class MenuModel
    \internal
*/

/*!
    \fn MenuModel::MenuModel( QObject* parent = 0 )
    \internal
*/

/*!
    \fn ServiceRequest* MenuModel::action( const QModelIndex& index, ActionContext context = Select ) const = 0
    \internal
*/

/*!
    \class SimpleMenuModel
    \internal
*/

/*!
    \fn SimpleMenuModel::~SimpleMenuModel()
    \internal
*/
SimpleMenuModel::~SimpleMenuModel()
{
    foreach( TextRequestPair item, m_items ) {
        delete item.request;
    }
}

/*!
    \fn SimpleMenuModel::addItem( const QString& text, const QIcon& icon, NavigationHint hint, const ServiceRequest& request )
    \internal
*/
void SimpleMenuModel::addItem( const QString& text, const QIcon& icon, NavigationHint hint, const ServiceRequest& request )
{
    TextRequestPair pair;
    pair.text = text;
    pair.icon = icon;
    pair.hint = hint;
    pair.request = request.clone();

    beginInsertRows( QModelIndex(), m_items.count(), m_items.count() );

    m_items.append( pair );

    endInsertRows();
}

/*!
    \fn SimpleMenuModel::addItem( const QString& text, const QIcon& icon, NavigationHint hint, QObject* receiver, const char* member )
    \internal
*/
void SimpleMenuModel::addItem( const QString& text, const QIcon& icon, NavigationHint hint, QObject* receiver, const char* member )
{
    TextRequestPair pair;
    pair.text = text;
    pair.icon = icon;
    pair.hint = hint;
    pair.request = new TriggerSlotRequest( receiver, member );

    beginInsertRows( QModelIndex(), m_items.count(), m_items.count() );

    m_items.append( pair );

    endInsertRows();
}

/*!
    \fn SimpleMenuModel::addItem( const QString& text, const QIcon& icon, NavigationHint hint, MenuModel* model )
    \internal
*/
void SimpleMenuModel::addItem( const QString& text, const QIcon& icon, NavigationHint hint, MenuModel* model )
{
    TextRequestPair pair;
    pair.text = text;
    pair.icon = icon;
    pair.hint = hint;

    PushMenuRequest pushmenu( model );
    PushTitleRequest pushtitle( text );
    pair.request = new CompoundRequest( QList<ServiceRequest*>() << &pushmenu << &pushtitle );

    beginInsertRows( QModelIndex(), m_items.count(), m_items.count() );

    m_items.append( pair );

    endInsertRows();
}

/*!
    \fn void SimpleMenuModel::addItem( const QString& text, const ServiceRequest& request )
    \internal
*/
void SimpleMenuModel::addItem( const QString& text, const ServiceRequest& request )
{
    addItem( text, QIcon(), NodeHint, request );
}

/*!
    \fn void SimpleMenuModel::addItem( const QString& text, QObject* receiver, const char* member )
    \internal
*/
void SimpleMenuModel::addItem( const QString& text, QObject* receiver, const char* member )
{
    addItem( text, QIcon(), NodeHint, receiver, member );
}

/*!
    \fn void SimpleMenuModel::addItem( const QString& text, MenuModel* model )
    \internal
*/
void SimpleMenuModel::addItem( const QString& text, MenuModel* model )
{
    addItem( text, QIcon(), NodeHint, model );
}

/*!
    \fn void SimpleMenuModel::removeRow( int row )
    \internal
*/
void SimpleMenuModel::removeRow( int row )
{
    beginRemoveRows( QModelIndex(), row, row );

    m_items.removeAt( row );

    endRemoveRows();
}

/*!
    \fn ServiceRequest* SimpleMenuModel::action( const QModelIndex&, ActionContext context ) const
    \internal
*/
ServiceRequest* SimpleMenuModel::action( const QModelIndex& index, ActionContext context ) const
{
    if( context == Select ) {
        TextRequestPair item = m_items[index.row()];

        return item.request->clone();
    }

    return 0;
}

/*!
    \fn QVariant SimpleMenuModel::data( const QModelIndex& index, int role ) const
    \internal
*/
QVariant SimpleMenuModel::data( const QModelIndex& index, int role ) const
{
    switch( role )
    {
    case Qt::DisplayRole:
        return m_items[index.row()].text;
    case Qt::DecorationRole:
        {
        QIcon icon = m_items[index.row()].icon;
        if( !icon.isNull() ) {
            return icon;
        }
        }
        break;
    case NAVIGATION_HINT_ROLE:
        return m_items[index.row()].hint;
    default:
        // Ignore
        break;
    }

    return QVariant();
}
