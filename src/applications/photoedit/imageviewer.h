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
#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QAbstractScrollArea>

class QContent;
class ImageViewerPrivate;

class ImageViewer : public QAbstractScrollArea
{
    Q_OBJECT
public:

    enum ScaleMode
    {
        FixedScale,
        ScaleToFit,
        ScaleRotateToFit
    };

    ImageViewer( QWidget *parent = 0 );
    virtual ~ImageViewer();

    QContent content() const;
    QByteArray format() const;
    QSize imageSize() const;
    bool canView() const;

    ScaleMode scaleMode() const;
    void setScaleMode( ScaleMode mode );

    qreal rotation() const;
    qreal scaleX() const;
    qreal scaleY() const;
    QSize scaledSize() const;
    QSize transformedSize() const;

signals:
    void imageInvalidated();

public slots:
    void setScale( qreal sx, qreal sy );
    void setRotation( qreal rotation );
    void setContent( const QContent &content );

protected:
    void paintEvent( QPaintEvent *event );
    void resizeEvent( QResizeEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void timerEvent( QTimerEvent *event );
    void scrollContentsBy( int dx, int dy );

private slots:
    void licenseExpired();
    void rectAvailable( const QRect &rect, const QImage &image );

private:
    ImageViewerPrivate *d;
};

#endif // IMAGEVIEWER_H
