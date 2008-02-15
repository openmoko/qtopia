/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef THUMBNAILMODEL_H
#define THUMBNAILMODEL_H

#include <QObject>
#include <QContentSet>
#include <QItemDelegate>
#include <QCache>
#include <QListView>

class QSize;
class QPixmap;
class QImage;
class QAbstractItemView;
class ThumbnailContentSetModelThread;

class ThumbnailContentSetModel : public QContentSetModel
{
    Q_OBJECT
public:
    ThumbnailContentSetModel( QContentSet *set, QObject *parent = 0 );
    ~ThumbnailContentSetModel();

    QSize thumbnailSize() const;
    void setThumbnailSize( const QSize &size );

    Qt::AspectRatioMode aspectRatioMode() const;
    void setAspectRatioMode( Qt::AspectRatioMode mode );

    QAbstractItemView *view() const;
    void setView( QAbstractItemView *view );

    QVariant data( const QModelIndex &index, int role ) const;

private slots:
    void thumbnailLoaded( const QImage &image );
    void contentChanged( const QContentIdList &contentIds, QContent::ChangeType type );

private:
    QPixmap thumbnail( const QModelIndex &index, const QContent &content ) const;

    QSize m_thumbnailSize;
    Qt::AspectRatioMode m_aspectRatioMode;
    mutable QCache< QContentId, QPixmap > m_cache;
    mutable QList< QPair< QPersistentModelIndex, QContent > > m_requestQueue;
    mutable ThumbnailContentSetModelThread *m_thread;
    QAbstractItemView *m_view;
};

class ContentThumbnailDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ContentThumbnailDelegate( QObject *parent = 0 );

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

class ContentThumbnailView : public QListView
{
    Q_OBJECT
public:
    ContentThumbnailView( QWidget *parent = 0 );

protected slots:
    void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );
    void rowsInserted( const QModelIndex &parent, int start, int end );
};

#endif
