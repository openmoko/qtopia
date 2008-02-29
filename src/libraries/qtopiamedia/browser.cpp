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

#include "browser.h"

PropertyBrowser::PropertyBrowser( Property property, const QContentFilter& filter, QObject* parent )
    : BrowserMenu( parent ), m_property( property ), m_filter( filter )
{
    populateData();
}

QString PropertyBrowser::filterString( const QModelIndex& index ) const
{
    return filterKey( m_property ) + m_values[index.row()];
}

QContentFilter PropertyBrowser::filter() const
{
    return m_filter;
}

void PropertyBrowser::setFilter( const QContentFilter& filter )
{
    m_filter = filter;

    populateData();
}

int PropertyBrowser::rowCount( const QModelIndex& ) const
{
    return m_values.count();
}

QVariant PropertyBrowser::data( const QModelIndex& index, int role ) const
{
    if( role == Qt::DisplayRole ) {
        return m_values[index.row()];
    }

    return QVariant();
}

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

ContentBrowser::ContentBrowser( const QContentFilter& filter, QObject* parent )
    : BrowserMenu( parent ), m_set( filter )
{
    m_setmodel = new QContentSetModel( &m_set );
}

ContentBrowser::~ContentBrowser()
{
    delete m_setmodel;
}

void ContentBrowser::setSortOrder( const QStringList& list )
{
    m_set.setSortOrder( list );
}

QContent ContentBrowser::content( const QModelIndex& index ) const
{
    return m_setmodel->content( index );
}

QContentFilter ContentBrowser::filter() const
{
    return m_set.filter();
}

void ContentBrowser::setFilter( const QContentFilter& filter )
{
    m_set.setCriteria( filter );
}

int ContentBrowser::rowCount( const QModelIndex& index ) const
{
    return m_setmodel->rowCount( index );
}

QVariant ContentBrowser::data( const QModelIndex& index, int role ) const
{
    return m_setmodel->data( index, role );
}

