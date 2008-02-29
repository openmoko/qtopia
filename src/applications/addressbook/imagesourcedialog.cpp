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

#include "imagesourcedialog.h"
#include <qtopiaapplication.h>
#include <qimagedocumentselector.h>
#include <qthumbnail.h>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QPainter>
#include <QButtonGroup>

#include <QDialog>
#include <QDesktopWidget>

#ifdef Q_WS_QWS
#include <QDSAction>
#include <QDSData>
#include <QDSServices>
#endif

class ImageSourceWidgetPrivate
{
public:
    ImageSourceWidgetPrivate()
    {
    }
    QPixmap pix;
    QLabel *photoLA;
    QPushButton *removePB, *changePB;
    int maxW;
    int maxH;
    QButtonGroup* editImageGroup;
};

ImageSourceWidget::ImageSourceWidget( QWidget *parent, const char *name )
:   QWidget( parent )
#ifdef Q_WS_QWS
,   getImageServices( 0 ),
    editImageServices( 0 )
#endif
{
    setObjectName(name);
    init();
    setPixmap( QPixmap() );
    haveImage( false );
}

ImageSourceWidget::ImageSourceWidget( const QPixmap &pix, QWidget * /* parent */ , const char * /*name*/ )
{
    init();
    setPixmap( pix );
}

ImageSourceWidget::~ImageSourceWidget()
{
    delete d;
}

void ImageSourceWidget::setMaximumImageSize(const QSize &s)
{
    d->maxW = s.width();
    d->maxH = s.height();
}

QSize ImageSourceWidget::maximumImageSize() const
{
    return QSize(d->maxW, d->maxH);
}

void ImageSourceWidget::init()
{
    d = new ImageSourceWidgetPrivate();

    d->maxW = 80;
    d->maxH = 96;

    QVBoxLayout *sl = new QVBoxLayout(this);

    QHBoxLayout *hl = new QHBoxLayout();
    sl->addLayout(hl);

    d->photoLA = new QLabel(0);
    d->photoLA->setAlignment(Qt::AlignCenter);
    d->photoLA->setFocusPolicy( Qt::NoFocus );
    hl->addWidget(d->photoLA);

    QVBoxLayout *vl = new QVBoxLayout();
    hl->addLayout(vl);

    sl->addStretch();

    d->changePB = new QPushButton(tr("Pictures"));
    vl->addWidget( d->changePB );
    connect( d->changePB, SIGNAL(clicked()), this, SLOT(change()) );

#ifdef Q_WS_QWS
    // Find all QDS services to get images
    getImageServices = new QDSServices( QString( "x-size/x-qsize" ),
                                        QString( "image/x-qpixmap" ),
                                        QStringList( QString( "get" ) ) );

    QButtonGroup* getImageGroup = new QButtonGroup( this );
    connect( getImageGroup,
             SIGNAL( buttonClicked ( int ) ),
             this,
             SLOT( getImageRequest( int ) ) );

    int getImageId = 0;
    foreach ( QDSServiceInfo serviceInfo, *getImageServices ) {
        if ( serviceInfo.isAvailable() ) {
            QPushButton* pb = 0;
            QIcon icon;
            if ( !serviceInfo.icon().isEmpty() ) {
                icon = QIcon( ":icon/" + serviceInfo.icon() );
            }

            if ( !icon.isNull() ) {
                pb = new QPushButton( /*icon,*/ serviceInfo.description() );
            } else {
                pb = new QPushButton( serviceInfo.description() );
            }

            vl->addWidget( pb );

            getImageGroup->addButton( pb, getImageId );
            ++getImageId;
        }
    }

    // Find all QDS services to edit images
    editImageServices = new QDSServices( QString( "image/x-qpixmap" ),
                                         QString( "image/x-qpixmap" ),
                                         QStringList( QString( "edit" ) ) );

    d->editImageGroup = new QButtonGroup( this );
    connect( d->editImageGroup,
             SIGNAL( buttonClicked ( int ) ),
             this,
             SLOT( editImageRequest( int ) ) );

    int editImageId = 0;
    foreach ( QDSServiceInfo serviceInfo, *editImageServices ) {
        QPushButton* pb = 0;
        QIcon icon;
        if ( !serviceInfo.icon().isEmpty() ) {
            icon = QIcon( ":icon/" + serviceInfo.icon() );
        }

        if ( !icon.isNull() ) {
            pb = new QPushButton( /*icon,*/ serviceInfo.description() );
        } else {
            pb = new QPushButton( serviceInfo.description() );
        }

        vl->addWidget( pb );

        d->editImageGroup->addButton( pb, editImageId );
        ++editImageId;
    }
#endif

    d->removePB = new QPushButton( tr("Remove"), this );
    connect( d->removePB, SIGNAL(clicked()), this, SLOT(remove()) );

    vl->addWidget( d->removePB );
    vl->addStretch();
}

void ImageSourceWidget::change()
{
    QImageDocumentSelectorDialog *s = new QImageDocumentSelectorDialog( this );
    s->setModal(true);
    if( QtopiaApplication::execDialog( s ) == QDialog::Accepted )
    {
        QThumbnail thumbnail( s->selectedDocument().file() );
        QPixmap newPix = thumbnail.pixmap( QSize( d->maxW, d->maxH ) );
        if( !newPix.isNull() )
        {
            setPixmap( newPix );
            haveImage( true );
        }
    }
    delete s;
}

void ImageSourceWidget::setPixmap( const QPixmap &pix )
{
    d->pix = pix;
    if( pix.isNull() )
    {
        haveImage( false );
        QFont f = font();
        f.setItalic( true );
        d->photoLA->setFont( f );
        d->photoLA->setText( tr("No Photo") );
    }
    else
    {
        QSize s( qMax(pix.width(), 100), qMax(pix.height(), 120) );
        if (d->photoLA->testAttribute(Qt::WA_Resized))
            s = QSize(qMin(d->photoLA->width(), s.width()),
                    qMin(d->photoLA->height(), s.height()));
        d->photoLA->setPixmap(pix.scaled(s, Qt::KeepAspectRatio));
        haveImage( true );
    }
}

QPixmap ImageSourceWidget::pixmap() const
{
    return d->pix;
}

void ImageSourceWidget::remove()
{
    setPixmap( QPixmap() );
}

void ImageSourceWidget::haveImage( bool f )
{
    d->removePB->setEnabled( f );
    foreach ( QAbstractButton* button, d->editImageGroup->buttons() )
        button->setEnabled( f );
}

void ImageSourceWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    QSize s( qMax(d->pix.width(), 100), qMax(d->pix.height(), 120) );
    s.scale( d->photoLA->width(), d->photoLA->height(), Qt::KeepAspectRatio );
    if ( !d->pix.isNull() )
        d->photoLA->setPixmap(d->pix.scaled(s));
}

void ImageSourceWidget::getImageRequest( int id )
{
    if ( getImageServices == 0 )
        return;

    QByteArray parametersArray;
    {
        QDataStream stream( &parametersArray, QIODevice::WriteOnly );
        stream << QSize( d->maxW, d->maxH );
    }

    QDSData parameters( parametersArray, QMimeType( "x-size/x-qsize" ) );
    QDSAction action( getImageServices->operator[]( id ) );
    if ( action.exec( parameters ) == QDSAction::CompleteData ) {
        QDataStream stream( action.responseData().toIODevice() );
        QPixmap image;
        stream >> image;
        setPixmap( image );
    } else {
        qWarning( "Unexpected QDS response" );
    }
}

void ImageSourceWidget::editImageRequest( int id )
{
    if ( editImageServices == 0 )
        return;

    QByteArray original;
    {
        QDataStream stream( &original, QIODevice::WriteOnly );
        stream << pixmap();
    }

    QDSData originalImage( original, QMimeType( "image/x-qpixmap" ) );
    QDSAction action( editImageServices->operator[]( id ) );
    if ( action.exec( originalImage ) == QDSAction::CompleteData ) {
        QDataStream stream( action.responseData().toIODevice() );
        QPixmap image;
        stream >> image;
        setPixmap( image );
    } else {
        qWarning( "Unexpected QDS response" );
    }
}

//dialog convenience wrapper
ImageSourceDialog::ImageSourceDialog( QWidget *parent, const char *name, bool modal, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setObjectName(name);
    setModal(modal);
    init();
}

ImageSourceDialog::ImageSourceDialog( const QPixmap &pix, QWidget *parent, const char *name, bool modal, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setObjectName(name);
    setModal(modal);
    init();
    setPixmap( pix );
}

void ImageSourceDialog::setMaximumImageSize(const QSize &s)
{
    mWidget->setMaximumImageSize(s);
}

QSize ImageSourceDialog::maximumImageSize() const
{
    return mWidget->maximumImageSize();
}

void ImageSourceDialog::init()
{
    QVBoxLayout *l = new QVBoxLayout( this );

    mWidget = new ImageSourceWidget( 0 );
    l->addWidget( mWidget );

    setWindowTitle( tr("Contact Photo") );
}

void ImageSourceDialog::setPixmap( const QPixmap &pix )
{
    mWidget->setPixmap( pix );
}

QPixmap ImageSourceDialog::pixmap() const
{
    return mWidget->pixmap();
}

