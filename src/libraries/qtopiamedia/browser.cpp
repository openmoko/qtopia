/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "browser_p.h"

/*!
    \class PropertyBrowser
    \internal
*/

/*!
    \fn PropertyBrowser::PropertyBrowser( Property property, const QContentFilter& filter, QObject* parent )
    \internal
*/
PropertyBrowser::PropertyBrowser( Property property, const QContentFilter& filter, QObject* parent )
    : BrowserMenu( parent ), m_property( property ), m_filter( filter )
{
    populateData();
}

/*!
    \fn QString PropertyBrowser::filterString( const QModelIndex& index ) const
    \internal
*/
QString PropertyBrowser::filterString( const QModelIndex& index ) const
{
    return filterKey( m_property ) + m_values[index.row()];
}

/*!
    \fn QContentFilter PropertyBrowser::filter() const
    \internal
*/
QContentFilter PropertyBrowser::filter() const
{
    return m_filter;
}

/*!
    \fn void PropertyBrowser::setFilter( const QContentFilter& filter )
    \internal
*/
void PropertyBrowser::setFilter( const QContentFilter& filter )
{
    m_filter = filter;

    populateData();
}

/*!
    \fn int PropertyBrowser::rowCount( const QModelIndex& ) const
    \internal
*/
int PropertyBrowser::rowCount( const QModelIndex& ) const
{
    return m_values.count();
}

/*!
    \fn QVariant PropertyBrowser::data( const QModelIndex& index, int role ) const
    \internal
*/
QVariant PropertyBrowser::data( const QModelIndex& index, int role ) const
{
    if( role == Qt::DisplayRole ) {
        return m_values[index.row()];
    }

    return QVariant();
}

/*!
    \fn QString PropertyBrowser::filterKey( Property property )
    \internal
*/
QString PropertyBrowser::filterKey( Property property )
{
    QString ret;

    switch( property )
    {
    case Artist:
        ret = "none/Artist/";
        break;
    case Album:
        ret = "none/Album/";
        break;
    case Genre:
        ret = "none/Genre/";
        break;
    default:
        // Unsupported property
        break;
    }

    return ret;
}

/*!
    \fn void PropertyBrowser::populateData()
    \internal
*/
void PropertyBrowser::populateData()
{
    m_values.clear();

    QString key = filterKey( m_property );
    QStringList args = m_filter.argumentMatches( QContentFilter::Synthetic, key );

    int keylen = key.length();
    foreach( QString arg, args ) {
        QString value = arg.mid( keylen );
        if( !value.isEmpty() ) {
            m_values.append( value );
        }
    }
}

/*!
    \class ContentBrowser
    \internal
*/

/*!
    \fn ContentBrowser::ContentBrowser( const QContentFilter& filter, QObject* parent )
    \internal
*/
ContentBrowser::ContentBrowser( const QContentFilter& filter, QObject* parent )
    : BrowserMenu( parent ), m_set( filter )
{
    m_setmodel = new QContentSetModel( &m_set );
}

/*!
    \fn ContentBrowser::~ContentBrowser()
    \internal
*/
ContentBrowser::~ContentBrowser()
{
    delete m_setmodel;
}

/*!
    \fn void ContentBrowser::setSortOrder( const QStringList& list )
    \internal
*/
void ContentBrowser::setSortOrder( const QStringList& list )
{
    m_set.setSortOrder( list );
}

/*!
    \fn QContent ContentBrowser::content( const QModelIndex& index ) const
    \internal
*/
QContent ContentBrowser::content( const QModelIndex& index ) const
{
    return m_setmodel->content( index );
}

/*!
    \fn QContentFilter ContentBrowser::filter() const
    \internal
*/
QContentFilter ContentBrowser::filter() const
{
    return m_set.filter();
}

/*!
    \fn void ContentBrowser::setFilter( const QContentFilter& filter )
    \internal
*/
void ContentBrowser::setFilter( const QContentFilter& filter )
{
    m_set.setCriteria( filter );
}

/*!
    \fn int ContentBrowser::rowCount( const QModelIndex& index ) const
    \internal
*/
int ContentBrowser::rowCount( const QModelIndex& index ) const
{
    return m_setmodel->rowCount( index );
}

/*!
    \fn QVariant ContentBrowser::data( const QModelIndex& index, int role ) const
    \internal
*/
QVariant ContentBrowser::data( const QModelIndex& index, int role ) const
{
    return m_setmodel->data( index, role );
}

