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

#ifndef MENUMODEL_H
#define MENUMODEL_H

#include <QtGui>
#include <qtopiaglobal.h>

class Playlist;

class QTOPIAMEDIA_EXPORT PlaylistModel
{
public:
    virtual ~PlaylistModel() { }

    // Return current playlist
    virtual Playlist* playlist() const = 0;
};

Q_DECLARE_INTERFACE(PlaylistModel,
    "com.trolltech.Qtopia.QtopiaMedia.PlaylistModel/1.0")

class ServiceRequest;

class QTOPIAMEDIA_EXPORT MenuModel : public QAbstractListModel
{
public:
    enum ActionContext { Select, Hold };

    explicit MenuModel( QObject* parent = 0 )
        : QAbstractListModel( parent )
    { }

    virtual ServiceRequest* action( const QModelIndex& index, ActionContext context = Select ) const = 0;
};

class QTOPIAMEDIA_EXPORT SimpleMenuModel : public MenuModel
{
public:
    explicit SimpleMenuModel( QObject* parent = 0 )
        : MenuModel( parent )
    { }

    ~SimpleMenuModel();

    void addItem( const QString& text, const ServiceRequest& request );
    void addItem( const QString& text, QObject* receiver, const char* member );
    void addItem( const QString& text, MenuModel* menu );

    // MenuModel
    ServiceRequest* action( const QModelIndex& index, ActionContext context = Select ) const;

    // AbstractListModel
    int rowCount( const QModelIndex& ) const { return m_items.count(); }
    QVariant data( const QModelIndex& index, int role ) const;

private:
    struct TextRequestPair
    {
        QString text;
        ServiceRequest *request;
    };

    QList<TextRequestPair> m_items;
};

#endif // MENUMODEL_H
