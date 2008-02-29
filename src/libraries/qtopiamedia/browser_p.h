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

#ifndef BROWSER_H
#define BROWSER_H

#include <private/menumodel_p.h>

#include <qcontentset.h>

class BrowserMenu : public MenuModel
{
public:
    explicit BrowserMenu( QObject* parent = 0 )
        : MenuModel( parent )
    { }

    virtual QContentFilter filter() const = 0;
    virtual void setFilter( const QContentFilter& filter ) = 0;
};

class QTOPIAMEDIA_EXPORT PropertyBrowser : public BrowserMenu
{
public:
    enum Property { Artist, Album, Genre };

    PropertyBrowser( Property property, const QContentFilter& filter = QContentFilter(), QObject* parent = 0 );

    Property property() const { return m_property; }

    // Return filter string for given row
    QString filterString( const QModelIndex& index ) const;

    // BrowserMenu
    QContentFilter filter() const;
    void setFilter( const QContentFilter& filter );

    // AbstractListModel
    int rowCount( const QModelIndex& = QModelIndex() ) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

protected:
    static QString filterKey( Property property );

private:
    void populateData();

    Property m_property;
    QContentFilter m_filter;
    QStringList m_args, m_values;
};

class QTOPIAMEDIA_EXPORT ContentBrowser : public BrowserMenu
{
public:
    explicit ContentBrowser( const QContentFilter& filter = QContentFilter(), QObject* parent = 0 );
    ~ContentBrowser();

    void setSortOrder( const QStringList& list );

    // Return QContent for given row
    QContent content( const QModelIndex& index ) const;

    // BrowserMenu
    QContentFilter filter() const;
    void setFilter( const QContentFilter& filter );

    // AbstractListModel
    int rowCount( const QModelIndex& index = QModelIndex() ) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

private:
    QContentSet m_set;
    QContentSetModel *m_setmodel;
};

#endif // BROWSER_H

