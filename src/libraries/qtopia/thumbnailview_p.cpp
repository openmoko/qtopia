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

#include "thumbnailview_p.h"

#include <qthumbnail.h>

#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#else
#include <QDateTime>
#include <QApplication>
#endif
#include <qsoftmenubar.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qlayout.h>

#include <QIcon>
#include <QImageReader>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QCursor>
#include <QDebug>
#include <QDesktopWidget>
#include <QContentSet>

// ### TODO: incorporate global pixmap cache

void ThumbnailCache::insert( const ThumbnailRequest& request, const QPixmap& pixmap )
{
    cache.insert( key( request ), new QPixmap( pixmap ) );
}

QPixmap ThumbnailCache::retrieve( const ThumbnailRequest& request )
{
    QPixmap ret;
    // Retrieve thumbnail from cache
    QPixmap *pixmap = cache.object( key( request ) );
    // If retrieve successful, return thumbnail
    // Otherwise, return null thumbnail
    if( pixmap ) {
        ret = *pixmap;
    }

    return ret;
}

bool ThumbnailCache::contains( const ThumbnailRequest &request ) const
{
    return cache.contains( key( request ) );
};

QString ThumbnailCache::key( const ThumbnailRequest& request )
{
     return request.filename() + QString::number( request.size().width() ) + QString::number( request.size().height() ) + request.time().toString( Qt::ISODate );
}

bool VisibleRule::isMetBy( const ThumbnailRequest& request ) const
{
    // If index visible in view, return true
    // Otherwise, return false
    QModelIndex index = request.index();
    if( index.isValid() && view_->visualRect( index ).intersects( view_->viewport()->contentsRect() ) ) {
        return true;
    }

    return false;
}

bool CacheRule::isMetBy( const ThumbnailRequest& request ) const
{
    // If thumbnail in cache, return true
    // Otherwise, return false
    return cache_->contains( request );
}

ThumbnailLoader::ThumbnailLoader( ThumbnailCache* cache, QObject* parent )
    : QObject( parent ), cache_( cache )
{
#define PAUSE 75
    connect( &load_timer, SIGNAL(timeout()), this, SLOT(loadFront()) );

    load_timer.setInterval( PAUSE );
    load_timer.setSingleShot( true );

    running = false;
}

void ThumbnailLoader::load( const ThumbnailRequest& request )
{
    // Enqueue thumbnail request
    queue.enqueue( request );
    // If load timer not started, start timer
    if( !running ) {
        running = true;
        load_timer.start();
    }
}

void ThumbnailLoader::loadFront()
{
    // Remove all items from front of queue that are not visible
    while( !queue.isEmpty() && ( cache_rule.isMetBy( queue.head() ) || !visible_rule.isMetBy( queue.head() )  )  ) {
        queue.dequeue();
    }

    // If queue is not empty, dequeue and load item
    if( !queue.isEmpty() ) {
        ThumbnailRequest request = queue.dequeue();
        // Load thumbnail and insert into cache

        QPixmap pixmap = loadThumbnail( request.filename(), request.size() );

        // Load thumbnail and insert into cache
        cache_->insert( request, pixmap );
        // Notify thumbnail has been loaded

        if( !pixmap.isNull() ) {
            emit loaded( request, pixmap );
        }
    }

    // If queue is not empty, restart timer
    if( !queue.isEmpty() ) {
        load_timer.start();
    } else {
        running = false;
        load_timer.stop();
    }
}

QPixmap ThumbnailLoader::loadThumbnail( const QString &filename, const QSize &size )
{
    QImageReader reader( filename );

    QImage image;

    bool scaled = false;

    if( reader.supportsOption( QImageIOHandler::Size ) && reader.supportsOption( QImageIOHandler::ScaledSize ) )
    {
        QSize maxSize = reader.size();

        maxSize.scale( size.boundedTo( reader.size() ), Qt::KeepAspectRatio );

        reader.setScaledSize( maxSize );

        scaled = true;
    }

    if( reader.read( &image ) )
    {
        if( !scaled )
        {
            QSize maxSize = size.boundedTo( image.size() );

            image = image.scaled( maxSize, Qt::KeepAspectRatio, Qt::FastTransformation );
        }

        return QPixmap::fromImage( image );
    }
    else
        return QPixmap();
}

ThumbnailRepository::ThumbnailRepository( ThumbnailCache* cache, ThumbnailLoader* loader, QObject* parent )
    : QObject( parent ), cache_( cache ), loader_( loader )
{
    connect( loader_, SIGNAL(loaded(ThumbnailRequest,QPixmap)),
        this, SIGNAL(loaded(ThumbnailRequest,QPixmap)) );
}

QPixmap ThumbnailRepository::thumbnail( const ThumbnailRequest& request )
{
    // If thumbnail available from cache, return thumbnail
    // Otherwise, request thumbnail from thumbnail loader
    // ### FIXME: continual loading of request if unable to load image
    QPixmap pixmap = cache_->retrieve( request );
    if( pixmap.isNull() && !cache_->contains( request ) ) {
        loader_->load( request );
    }

    return pixmap;
}

void ThumbnailDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_ASSERT(index.isValid());
    const QContentSetModel *model = qobject_cast< const QContentSetModel * >( index.model() );
    Q_ASSERT(model);

    QStyleOptionViewItem opt = option;

    // set text alignment
    opt.displayAlignment = Qt::AlignCenter;

    // do layout
    QContent content = model->content( index );
    QPixmap pixmap;

    if( content.fileKnown() ) {
        pixmap = repository_->thumbnail( ThumbnailRequest( index, content.file(), option.decorationSize, content.lastUpdated() ) );
    }
    if( pixmap.isNull() ) {
        QIcon icon = qvariant_cast<QIcon>(model->data(index, Qt::DecorationRole));
        pixmap = icon.pixmap( option.decorationSize );
    }
    QRect pixmapRect = QRect(0, 0, option.decorationSize.width(),
                           option.decorationSize.height());

    QFontMetrics fontMetrics(opt.font);
    QString text = model->data(index, Qt::DisplayRole).toString();
    QRect textRect(0, 0, option.decorationSize.width(), fontMetrics.lineSpacing());

    QVariant value = model->data(index, Qt::CheckStateRole);
    QRect checkRect = check(opt, opt.rect, value);
    Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());

    doLayout(opt, &checkRect, &pixmapRect, &textRect, false);

    // draw the background color
    if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    } else {
        value = model->data(index, Qt::BackgroundColorRole);
        if (value.isValid() && qvariant_cast<QColor>(value).isValid())
            painter->fillRect(option.rect, qvariant_cast<QColor>(value));
    }

    // draw the item
    drawCheck(painter, opt, checkRect, checkState);
    drawDecoration(painter, opt, pixmapRect, pixmap);
    drawDisplay(painter, opt, textRect, text);
    drawFocus(painter, opt, textRect);
}

QSize ThumbnailDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_ASSERT(index.isValid());
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    QFont fnt = option.font;
    QString text = model->data(index, Qt::DisplayRole).toString();
    QRect pixmapRect = QRect(0, 0, option.decorationSize.width(),
                           option.decorationSize.height());

    QFontMetrics fontMetrics(fnt);
    QRect textRect(0, 0, option.decorationSize.width(), fontMetrics.lineSpacing());
    QRect checkRect = check(option, textRect, model->data(index, Qt::CheckStateRole));
    doLayout(option, &checkRect, &pixmapRect, &textRect, true);

    return (pixmapRect|textRect|checkRect).size();
}

ThumbnailView::ThumbnailView( QWidget* parent )
    : QListView( parent )
{
    QDesktopWidget *desktop = QApplication::desktop();
    resize(desktop->availableGeometry(desktop->screenNumber(this)).width(), height());  //### workaround to make view layout properly on first view.
    setMovement( QListView::Static );
    setFrameStyle(QFrame::NoFrame);
    connect( this, SIGNAL(pressed(QModelIndex)), this, SLOT(emitSelected(QModelIndex)) );
}

void ThumbnailView::repaintThumbnail( const ThumbnailRequest& request )
{
    dataChanged( request.index(), request.index() );
}

void ThumbnailView::emitSelected( const QModelIndex& index )
{
    // If item selected, emit selected
    if( index.isValid() ) {
        emit selected();
    }
}

void ThumbnailView::keyPressEvent( QKeyEvent* e )
{
    QListView::keyPressEvent( e );
    switch( e->key() ) {
    // If select key, emit selected signal
    case Qt::Key_Select:
    case Qt::Key_Enter:
        emit selected();
        break;
    default:
        // Ignore
        break;
    }

}
