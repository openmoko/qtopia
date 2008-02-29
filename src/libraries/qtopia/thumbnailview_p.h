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

#ifndef THUMBNAILVIEW_P_H
#define THUMBNAILVIEW_P_H

#include <qlistwidget.h>
#include <qitemdelegate.h>
#include <qqueue.h>
#include <qtimer.h>

#include <QItemDelegate>
#include <QCache>

class ThumbnailRequest
{
public:
    ThumbnailRequest( const QModelIndex& index, const QString& filename, const QSize& size )
        : index_( index ), filename_( filename ), size_( size )
    { }

    QModelIndex index() const { return index_; }

    QString filename() const { return filename_; }

    QSize size() const { return size_; }

private:
    QPersistentModelIndex index_;
    QString filename_;
    QSize size_;
};

class ThumbnailCache : public QObject
{
    Q_OBJECT
public:
    // ### TODO: set cache size
    explicit ThumbnailCache( QObject* parent )
        : QObject( parent )
    { }

    // Add thumbnail to cache
    void insert( const ThumbnailRequest& request, const QPixmap& pixamp );

    // Return thumbnail if in cache
    QPixmap retrieve( const ThumbnailRequest& request );

private:
    static QString key( const ThumbnailRequest& request );

    QCache<QString, QPixmap> cache;
};

class ThumbnailView;

class VisibleRule
{
public:
    explicit VisibleRule( ThumbnailView* view = 0 )
        : view_( view )
    { }

    // Return true if thumbnail is currently visible in view
    bool isMetBy( const ThumbnailRequest& ) const;

private:
    ThumbnailView *view_;
};

class CacheRule
{
public:
    explicit CacheRule( ThumbnailCache* cache = 0 )
        : cache_( cache )
    { }

    // Return true if thumbnail is currently in cache
    bool isMetBy( const ThumbnailRequest& ) const;

private:
    ThumbnailCache *cache_;
};

class ThumbnailLoader : public QObject
{
    Q_OBJECT
public:
    ThumbnailLoader( ThumbnailCache* cache, QObject* parent );
    virtual ~ThumbnailLoader(){}

    // Add request to load queue
    void load( const ThumbnailRequest& );

    // ### TODO: could be generalized into load rule
    void setVisibleRule( const VisibleRule& rule ) { visible_rule = rule; }

    // ### TODO: could be generalized into load rule
    void setCacheRule( const CacheRule& rule ) { cache_rule = rule; }

signals:
    // Thumbnail request loaded into cache
    void loaded( const ThumbnailRequest& request, const QPixmap& thumbnail );

private slots:
    // Load thumbnail item at front of queue into cache
    void loadFront();

protected:
    virtual QPixmap loadThumbnail( const QString &filename, const QSize &size );

private:
    bool running;
    QTimer load_timer;
    CacheRule cache_rule;
    VisibleRule visible_rule;
    QQueue<ThumbnailRequest> queue;
    ThumbnailCache *cache_;
};

class ThumbnailRepository : public QObject
{
    Q_OBJECT
public:
    ThumbnailRepository( ThumbnailCache* cache, ThumbnailLoader* loader, QObject* parent );

    // Return thumbnail if in cache
    QPixmap thumbnail( const ThumbnailRequest& request );

signals:
    // Thumbnail request loaded into cache
    void loaded( const ThumbnailRequest& request, const QPixmap& thumbnail );

private:
    ThumbnailCache *cache_;
    ThumbnailLoader *loader_;
};

class ThumbnailDelegate : public QItemDelegate
{
public:
    ThumbnailDelegate( ThumbnailRepository* repository, QObject* parent )
        : QItemDelegate( parent ), repository_( repository )
    { }

    void paint( QPainter*, const QStyleOptionViewItem&, const QModelIndex& ) const;

    QSize sizeHint( const QStyleOptionViewItem&, const QModelIndex& ) const;

private:
    ThumbnailRepository *repository_;
};

class ThumbnailView : public QListView
{
    Q_OBJECT
public:
    explicit ThumbnailView( QWidget* parent=0 );

signals:
    // Select key pressed
    void selected();

    // Only Qtopia PDA
    // ### TODO: stylus held on thumbnail
    void held( const QPoint& );

public slots:
    void repaintThumbnail( const ThumbnailRequest& request );

private slots:
    void emitSelected( const QModelIndex& index );

protected:
    void keyPressEvent( QKeyEvent* );
};

#endif // THUMBNAILVIEW_P_H
