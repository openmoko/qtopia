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

#include "qwaitwidget.h"
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHideEvent>
#include <qtopiaapplication.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

class QWaitWidgetPrivate
{
public:
    QTimer *intervalTimer;
    QLabel *textLabel;
    QLabel *imageLabel;
    QPixmap busyPixmap;
    int count;
    int w, h;
    bool cancelEnabled;
    bool wasCancelled;
    int expiryTime;
};

/*!
   Construct a QWaitWidget to popup over \a parent
   \code
   QWaitWidget *wl = new QWaitWidget(this);
   wl->show();
   // do time consuming operations
   delete wl;
   \endcode
*/


QWaitWidget::QWaitWidget(QWidget *parent)
    : QDialog( parent )
{
    d = new QWaitWidgetPrivate;

    d->count = 0;
    d->intervalTimer = new QTimer( this );
    d->intervalTimer->setInterval( 100 );
    connect( d->intervalTimer, SIGNAL(timeout()),
            this, SLOT(updateImage()) );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 2 );
    layout->setSpacing( 2 );

    d->textLabel = new QLabel( this );
    d->textLabel->setWordWrap( true );
    d->textLabel->setAlignment(Qt::AlignHCenter);
    d->imageLabel = new QLabel( this );

    layout->addStretch( 0 );
    layout->addWidget( d->imageLabel );
    layout->addWidget( d->textLabel );
    layout->addStretch( 0 );
    layout->setAlignment( d->textLabel, Qt::AlignHCenter | Qt::AlignVCenter );
    layout->setAlignment( d->imageLabel, Qt::AlignHCenter | Qt::AlignVCenter );

    setWindowTitle( tr( "Please wait" ) );
    setModal( true );
    setWindowFlags( Qt::SplashScreen );

    QColor col = QApplication::palette().color( QPalette::Active, QPalette::Highlight );
    setColor( col );

#ifdef QTOPIA_PHONE
    QSoftMenuBar::removeMenuFrom( this, QSoftMenuBar::menuFor( this ) );
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::NoLabel );
    d->cancelEnabled = false;
    d->wasCancelled = false;
#endif
    d->expiryTime = 0;
}

/*!
    Makes the Cancel button appear on the context menu.
    When the Cancel button is presed the signal cancelled() is emitted.
*/
void QWaitWidget::setCancelEnabled(bool enabled)
{
#ifdef QTOPIA_PHONE
    if (enabled) {
        d->cancelEnabled = true;
        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );
    }
    else {
        d->cancelEnabled = false;
        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::NoLabel );
    }
#endif
}

/*!
    Returns whether the widget was cancelled by the user when it was last
    shown.
 */
bool QWaitWidget::wasCancelled() const
{
    return d->wasCancelled;
}

/*!
    Hides the widget after \a msec.
*/
void QWaitWidget::setExpiryTime( int msec )
{
    d->expiryTime = msec;
}

/*!
  Shows the widget and its child widgets. The widget is shown maximized.
*/
void QWaitWidget::show()
{
    if ( d->expiryTime != 0 )
        QTimer::singleShot( d->expiryTime, this, SLOT(timeExpired()) );
    d->intervalTimer->start();
    QDialog::showMaximized();

    d->wasCancelled = false;
}

/*!
  Hides the widget and its child widgets.
*/
void QWaitWidget::hide()
{
    reset();
    QDialog::hide();
}

void QWaitWidget::updateImage()
{
    if ( d->count == 16 )
        d->count = 0;
    QPixmap copy = d->busyPixmap.copy( d->count * d->w, 0, d->w, d->h );
    d->imageLabel->setPixmap( copy );
    d->count++;
}

void QWaitWidget::timeExpired()
{
    hide();
}

/*
   Sets the text label to text \a str.
*/
void QWaitWidget::setText( const QString &str )
{
    d->textLabel->setText( str );
}

/*
   Blends the image with color \a col.
*/
void QWaitWidget::setColor( const QColor &col )
{
    int sr, sg, sb;
    col.getRgb( &sr, &sg, &sb );

    QImage img( ":image/busy" );

    if ( img.depth() == 32 ) {
        QRgb *rgb = (QRgb *)img.bits();
        int bytesCount = img.bytesPerLine()/sizeof(QRgb)*img.height();
        for ( int r = 0; r < bytesCount; r++, rgb++ )
            *rgb = blendRgb( *rgb, sr, sg, sb );
    } else {
        QVector<QRgb> rgb = img.colorTable();
        for ( int r = 0; r < rgb.count(); r++ )
            rgb[r] = blendRgb( rgb[r], sr, sg, sb );
        img.setColorTable( rgb );
    }

    d->busyPixmap = QPixmap::fromImage( img );
    d->w = d->busyPixmap.width() / 16;
    d->h = d->busyPixmap.height();

    updateImage();
}

QRgb QWaitWidget::blendRgb( QRgb rgb, int sr, int sg, int sb )
{
    int tmp = ( rgb >> 16 ) & 0xff;
    int r = ( ( sr + tmp ) / 2 );
    tmp = ( rgb >> 8 ) & 0xff;
    int g = ( ( sg + tmp ) / 2 );
    tmp = rgb & 0xff;
    int b = ( ( sb + tmp ) / 2 );
    return qRgba( r, g, b, qAlpha( rgb ) );
}

void QWaitWidget::hideEvent( QHideEvent *e )
{
    reset();
    QDialog::hideEvent( e );
}

void QWaitWidget::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Back && d->cancelEnabled ) {
        d->wasCancelled = true;
        emit cancelled();
        QDialog::keyPressEvent( e );
    } else {
        e->accept();
    }
}

void QWaitWidget::reset()
{
    if ( d->intervalTimer->isActive() )
        d->intervalTimer->stop();
    d->count = 0;
}

