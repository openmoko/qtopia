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
#include "imageviewer.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QPainter>
#include <QImageReader>
#include <QPaintEvent>
#include <QScrollBar>
#include <QtDebug>
#include <QDrmContent>

class ImageViewerLoader : public QObject
{
    Q_OBJECT
public:
    ImageViewerLoader( ImageViewerPrivate *viewer_d );

public slots:
    void setContent( const QContent &content );
    void fetchRect( const QRect &rect, const QRect &source );

signals:
    void rectAvailable( const QRect &rect, const QImage &image );

private:
    ImageViewerPrivate *d;
    QImage m_image;
};

class ImageViewerPrivate : public QThread
{
    Q_OBJECT
public:

    ImageViewerPrivate( ImageViewer *viewer );

    ImageViewer *q;
    QContent content;
    QDrmContent drmContent;
    QByteArray format;
    QSize size;
    bool canView;
    ImageViewer::ScaleMode scaleMode;
    qreal prescaling;
    qreal scaleX;
    qreal scaleY;
    qreal rotation;
    QSize scaledSize;
    QSize transformedSize;
    QRect screenRect;
    QPixmap pixmap;
    QRect pixmapRect;
    int updateTimerId;
    QPoint lastMousePos;
    QMutex syncMutex;
    QWaitCondition syncCondition;

    void calculateScale();
    void calculateTransform();
    void calculateScreenRect();

signals:
    void setContent( const QContent &content );
    void fetchRect( const QRect &rect, const QRect &source );

protected:
    void run();

    friend class ImageViewer;
};

ImageViewerLoader::ImageViewerLoader( ImageViewerPrivate *viewer_d )
    : d( viewer_d )
{
}

void ImageViewerLoader::setContent( const QContent &content )
{
    static const int maxArea = 6144000;

    m_image = QImage();

    d->content = content;

    QIODevice *device = 0;
    QImageReader reader;

    d->format     = QByteArray();
    d->size       = QSize();
    d->prescaling = 1.0;
    d->canView    = false;

    int area = 0;

    QDrmContent drmContent( QDrmRights::Display, QDrmContent::NoLicenseOptions );

    if( !content.isNull() && drmContent.requestLicense( content ) && (device = content.open()) != 0 )
    {
        reader.setDevice( device );

        d->format     = reader.format();
        d->size       = reader.size();
        d->prescaling = 1.0;

        area = d->size.width() * d->size.height();

        d->canView  = reader.canRead() && area <= maxArea || reader.supportsOption( QImageIOHandler::ScaledSize );
    }

    while( d->canView && area > maxArea )
    {
        d->prescaling /= 2.0;

        area = qRound( area * d->prescaling * d->prescaling );
    }

    {
        QMutexLocker locker( &d->syncMutex );

        d->syncCondition.wakeAll();
    }

    if( d->canView )
    {
        if( d->prescaling < 1.0 )
            reader.setScaledSize( d->size * d->prescaling );

        reader.read( &m_image );
    }

    if( device )
        device->close();

    delete device;
}

void ImageViewerLoader::fetchRect( const QRect &rect, const QRect &source )
{
    if( source.size() == m_image.size() )
    {
        QImage transformedImage = m_image.scaled( rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation );

        emit rectAvailable( rect, transformedImage );
    }
    else
    {
        QImage transformedImage( rect.size(), m_image.format() );

        {
            QPainter painter( &transformedImage );

            painter.setRenderHint( QPainter::Antialiasing );
            painter.setRenderHint( QPainter::SmoothPixmapTransform );

            painter.drawImage( QRect( QPoint( 0, 0 ), rect.size() ), m_image, source );

            painter.end();
        }

        emit rectAvailable( rect, transformedImage );
    }
}

ImageViewerPrivate::ImageViewerPrivate( ImageViewer *viewer )
    : q( viewer )
    , drmContent( QDrmRights::Display )
    , canView( false )
    , scaleMode( ImageViewer::ScaleToFit )
    , prescaling( 1.0 )
    , scaleX( 1.0 )
    , scaleY( 1.0 )
    , rotation( 0.0 )
    , updateTimerId( -1 )
{
}

void ImageViewerPrivate::calculateScale()
{
    if( size.isValid() )
    {
        QSize bestFitSize = size;
        bestFitSize.scale( q->size(), Qt::KeepAspectRatio );

        qreal scale = qMin( qreal(1), qreal(bestFitSize.width()) / size.width() );

        rotation = 0.0;

        if( scaleMode == ImageViewer::ScaleRotateToFit )
        {
            bestFitSize = size;
            bestFitSize.transpose();
            bestFitSize.scale( q->size(), Qt::KeepAspectRatio );

            qreal rotatedScale = qMin( qreal(1), qreal(bestFitSize.width()) / size.height() );

            if( rotatedScale > scale )
            {
                scale = rotatedScale;

                rotation = -90.0;
            }
        }

        scaleX = scale;
        scaleY = scale;
    }
    else
    {
        scaleX   = 1.0;
        scaleY   = 1.0;
        rotation = 0.0;
    }

    calculateTransform();
}

void ImageViewerPrivate::calculateTransform()
{
    QRect imageRect( QPoint( 0, 0 ), size );

    QTransform transform;

    transform.scale( scaleX, scaleY );

    scaledSize = transform.mapRect( imageRect ).size();

    transform.rotate( rotation );

    QSize oldTransformedSize = transformedSize;

    transformedSize = transform.mapRect( imageRect ).size();

    QScrollBar *hScroll = q->horizontalScrollBar();
    QScrollBar *vScroll = q->verticalScrollBar();

    if( canView )
    {
        int hValue = hScroll->value();
        int vValue = vScroll->value();

        QSize dSize = transformedSize - oldTransformedSize;

        hValue += dSize.width() / 2;
        vValue += dSize.height() / 2;

        dSize = transformedSize - q->size();

        hScroll->setRange( 0, dSize.width() );
        vScroll->setRange( 0, dSize.height() );
        hScroll->setValue( hValue );
        vScroll->setValue( vValue );
    }
    else
    {
        hScroll->setRange( 0, 0 );
        vScroll->setRange( 0, 0 );
    }

    calculateScreenRect();
}

void ImageViewerPrivate::calculateScreenRect()
{
    if( canView )
    {
        QTransform transform;

        transform.translate( -q->horizontalScrollBar()->value() + qMax( transformedSize.width(),  q->width() )  / 2,
                             -q->verticalScrollBar()->value()   + qMax( transformedSize.height(), q->height() ) / 2 );
        transform.rotate( rotation );
        transform.translate( -scaledSize.width() / 2, -scaledSize.height() / 2 );

        QTransform inverse = transform.inverted();

        QRect untransformedRect( 0, 0, q->width(), q->height() );

        screenRect = inverse.mapRect( untransformedRect );

        screenRect = screenRect.intersected( QRect( QPoint( 0, 0 ), scaledSize ) );
    }
    else
        screenRect = QRect();
}

void ImageViewerPrivate::run()
{
    ImageViewerLoader loader( this );

    connect( this,  SIGNAL(setContent(QContent)),
             &loader, SLOT(setContent(QContent)) );

    connect( this,  SIGNAL(fetchRect(QRect,QRect)),
             &loader, SLOT(fetchRect(QRect,QRect)) );

    connect( &loader, SIGNAL(rectAvailable(QRect,QImage)),
             q,         SLOT(rectAvailable(QRect,QImage)) );

    {
        QMutexLocker locker( &syncMutex );

        syncCondition.wakeAll();
    }

    exec();
}

ImageViewer::ImageViewer( QWidget *parent )
    : QAbstractScrollArea( parent )
    , d( new ImageViewerPrivate( this ) )
{
    static const int qRectMetaId = qRegisterMetaType<QRect>();

    Q_UNUSED( qRectMetaId );

    d->drmContent.setFocusWidget( this );

    connect( &d->drmContent, SIGNAL(rightsExpired(QDrmContent)), this, SLOT(licenseExpired()) );

    setMinimumSize( 32, 32 );
    setFrameStyle( QFrame::NoFrame );

    horizontalScrollBar()->setSingleStep( 10 );
    verticalScrollBar()->setSingleStep( 10 );

    QMutexLocker locker( &d->syncMutex );

    d->start();

    d->syncCondition.wait( &d->syncMutex );
}

ImageViewer::~ImageViewer()
{
    d->quit();

    d->wait();

    delete d;
}

void ImageViewer::setContent( const QContent &content )
{
    d->pixmap = QPixmap();
    d->pixmapRect = QRect();

    QContent c;

    if( content.isNull() )
        d->drmContent.releaseLicense();
    else if( d->drmContent.requestLicense( content ) )
        c = content;

    {
        QMutexLocker locker( &d->syncMutex );

        d->setContent( c );

        d->syncCondition.wait( &d->syncMutex );
    }

    horizontalScrollBar()->setRange( 0, 0 );
    verticalScrollBar()->setRange( 0, 0 );

    if( d->scaleMode != FixedScale )
        d->calculateScale();
    else
        d->calculateTransform();

    if( d->canView && d->updateTimerId == -1 )
        d->updateTimerId = startTimer( 0 );

    viewport()->update();
}

QContent ImageViewer::content() const
{
    return d->content;
}

QByteArray ImageViewer::format() const
{
    return d->format;
}

QSize ImageViewer::imageSize() const
{
    return d->size;
}

qreal ImageViewer::scaleX() const
{
    return d->scaleX;
}

qreal ImageViewer::scaleY() const
{
    return d->scaleY;
}

QSize ImageViewer::scaledSize() const
{
    return d->scaledSize;
}

QSize ImageViewer::transformedSize() const
{
    return d->transformedSize;
}

void ImageViewer::setScale( qreal sx, qreal sy )
{
    d->scaleMode = FixedScale;

    if( sx != d->scaleX && d->scaleY != sy  )
    {
        if( d->scaleX != 0.0 && d->scaleY != 0.0 && d->pixmapRect.isValid() )
        {
            QTransform transform;

            transform.scale( sx / d->scaleX, sy / d->scaleY );

            d->pixmapRect = transform.mapRect( d->pixmapRect );

            viewport()->update();
        }

        d->scaleX = sx;
        d->scaleY = sy;

        d->calculateTransform();

        if( d->updateTimerId == -1 && d->canView )
            d->updateTimerId = startTimer( 0 );
    }
}

qreal ImageViewer::rotation() const
{
    return d->rotation;
}

void ImageViewer::setRotation( qreal rotation )
{
    d->scaleMode = FixedScale;

    if( rotation != d->rotation )
    {
        d->rotation = rotation;

       d->calculateTransform();

        if( d->updateTimerId == -1 && d->canView )
            d->updateTimerId = startTimer( 0 );

        viewport()->update();
    }
}

ImageViewer::ScaleMode ImageViewer::scaleMode() const
{
    return d->scaleMode;
}

void ImageViewer::setScaleMode( ScaleMode mode )
{
    d->scaleMode = mode;

    if( mode != FixedScale )
    {
        d->calculateScale();

        if( d->updateTimerId == -1 && d->canView )
            d->updateTimerId = startTimer( 0 );

        viewport()->update();
    }
}

void ImageViewer::paintEvent( QPaintEvent *event )
{
    if( d->canView )
    {
        QPainter painter( viewport() );

        painter.setClipRegion( event->region() );

        QTransform transform;

        transform.translate( -horizontalScrollBar()->value() + qMax( d->transformedSize.width(),  width() )  / 2,
                             -verticalScrollBar()->value()   + qMax( d->transformedSize.height(), height() ) / 2 );
        transform.rotate( d->rotation );
        transform.translate( -d->scaledSize.width() / 2, -d->scaledSize.height() / 2 );

        painter.setWorldTransform( transform, true );

        if( !d->pixmap.isNull() && d->pixmapRect.intersects( d->screenRect ) )
        {
            if( !d->pixmapRect.contains( d->screenRect ) )
            {
                QBrush brush( palette().windowText().color(), Qt::DiagCrossPattern );

                QRect unfilledRect = d->screenRect;

                if( unfilledRect.left() < d->pixmapRect.left() )
                {
                    QRect hatchRect( 
                        unfilledRect.left(),
                        unfilledRect.top(),
                        d->pixmapRect.left() - unfilledRect.left(),
                        unfilledRect.height() );

                    painter.fillRect( hatchRect, brush );

                    unfilledRect.adjust( hatchRect.width(), 0, 0, 0 );
                }
                if( unfilledRect.top() < d->pixmapRect.top() )
                {
                    QRect hatchRect( 
                        unfilledRect.left(),
                        unfilledRect.top(),
                        unfilledRect.width(),
                        d->pixmapRect.top() - unfilledRect.top() );

                    painter.fillRect( hatchRect, brush );

                    unfilledRect.adjust( 0, hatchRect.height(), 0, 0 );
                }
                if( unfilledRect.right() > d->pixmapRect.right() )
                {
                    QRect hatchRect( 
                        d->pixmapRect.right(),
                        unfilledRect.top(),
                        unfilledRect.right() - d->pixmapRect.right(),
                        unfilledRect.height() );

                    painter.fillRect( hatchRect, brush );

                    unfilledRect.adjust( 0, 0, -hatchRect.width(), 0 );
                }
                if( unfilledRect.bottom() > d->pixmapRect.bottom() )
                {
                    QRect hatchRect( 
                        unfilledRect.left(),
                        d->pixmapRect.bottom(),
                        unfilledRect.width(),
                        unfilledRect.bottom() - d->pixmapRect.bottom());

                    painter.fillRect( hatchRect, brush );
                }
            }

            painter.drawPixmap( d->pixmapRect, d->pixmap );
        }
        else
        {
            painter.fillRect( d->screenRect, QBrush( palette().windowText().color(), Qt::DiagCrossPattern ) );
        }

        event->accept();
    }
    else
        QAbstractScrollArea::paintEvent( event );
}

void ImageViewer::resizeEvent( QResizeEvent *event )
{
    QScrollBar *hScroll = horizontalScrollBar();
    QScrollBar *vScroll = verticalScrollBar();

    int hValue = hScroll->value();
    int vValue = vScroll->value();

    QSize dSize = d->transformedSize - event->size();

    hScroll->setRange( 0, dSize.width() );
    vScroll->setRange( 0, dSize.height() );

    dSize = event->size() - event->oldSize();

    hScroll->setValue( hValue + dSize.width()  / 2 );
    vScroll->setValue( vValue + dSize.height() / 2 );

    hScroll->setPageStep( width()  );
    vScroll->setPageStep( height() );

    if( d->scaleMode != FixedScale )
        d->calculateScale();
    else
        d->calculateScreenRect();

    QAbstractScrollArea::resizeEvent( event );
}

void ImageViewer::mousePressEvent( QMouseEvent *event )
{ 
    QAbstractScrollArea::mousePressEvent( event );

    if( event->button() == Qt::LeftButton )
        d->lastMousePos = event->pos();
}

void ImageViewer::mouseMoveEvent( QMouseEvent *event )
{
    QAbstractScrollArea::mouseMoveEvent( event );

    if( !d->lastMousePos.isNull() )
    {
        QPoint dPos = event->pos() - d->lastMousePos;

        QScrollBar *hScroll = horizontalScrollBar();
        QScrollBar *vScroll = verticalScrollBar();

        hScroll->setValue( hScroll->value() - dPos.x() );
        vScroll->setValue( vScroll->value() - dPos.y() );

        d->lastMousePos = event->pos();
    }
}

void ImageViewer::mouseReleaseEvent( QMouseEvent *event )
{
    QAbstractScrollArea::mouseReleaseEvent( event );

    if( event->button() == Qt::LeftButton )
        d->lastMousePos = QPoint();
}

void ImageViewer::timerEvent( QTimerEvent *event )
{
    if( event->timerId() == d->updateTimerId )
    {
        killTimer( event->timerId() );

        d->updateTimerId = -2;

        int maxDimension = qMax( width(), height() ) * 3;

        QSize sectionSize( 
                qMin( maxDimension,  d->scaledSize.width() ),
                qMin( maxDimension, d->scaledSize.height() ) );

        QPoint sectionPos(
                qMax( qMin( d->scaledSize.width()  - sectionSize.width(),  d->screenRect.x() - 2 * width()  / 2 ), 0 ),
                qMax( qMin( d->scaledSize.height() - sectionSize.height(), d->screenRect.y() - 2 * height() / 2 ), 0 ) );

        QRect sectionRect( sectionPos, sectionSize );

        QTransform transform;
        transform.scale( d->prescaling / d->scaleX, d->prescaling / d->scaleY );

        QRect sourceRect = transform.mapRect( sectionRect );

        emit d->fetchRect( sectionRect, sourceRect );

        event->accept();
    }
    else
        QAbstractScrollArea::timerEvent( event );
}

void ImageViewer::scrollContentsBy( int dx, int dy )
{
    QAbstractScrollArea::scrollContentsBy( dx, dy );

    d->calculateScreenRect();

    if( d->updateTimerId == -1 )
    {
        QRect borderRect = d->screenRect;

        borderRect.adjust( -20, -20, 20, 20 );

        borderRect = borderRect.intersected( QRect( QPoint( 0, 0 ), d->scaledSize ) );

        if( borderRect.isValid() && (
            borderRect.top()    < d->pixmapRect.top()    || 
            borderRect.left()   < d->pixmapRect.left()   ||
            borderRect.bottom() > d->pixmapRect.bottom() ||
            borderRect.right()  > d->pixmapRect.right()  ) )
            d->updateTimerId = startTimer( 0 );
    }
}

void ImageViewer::licenseExpired()
{
    setContent( QContent() );

    emit imageInvalidated();
}

void ImageViewer::rectAvailable( const QRect &rect, const QImage &image )
{
    if( !image.isNull() )
    {
        if( d->drmContent.renderState() != QDrmContent::Started )
            d->drmContent.renderStarted();

        d->pixmap = QPixmap();
        d->pixmap = QPixmap::fromImage( image );
        d->pixmapRect = rect;

        QRect borderRect = d->screenRect;

        borderRect.adjust( -20, -20, 20, 20 );

        borderRect = borderRect.intersected( QRect( QPoint( 0, 0 ), d->scaledSize ) );

        if( borderRect.isValid() && (
            borderRect.top()    < d->pixmapRect.top()    || 
            borderRect.left()   < d->pixmapRect.left()   ||
            borderRect.bottom() > d->pixmapRect.bottom() ||
            borderRect.right()  > d->pixmapRect.right()  ) )
            d->updateTimerId = startTimer( 0 );
        else
            d->updateTimerId = -1;

    }
    else
    {
        d->pixmap = QPixmap();
        d->pixmapRect = QRect();

        d->updateTimerId = -1;
    }

    viewport()->update();
}

#include "imageviewer.moc"
