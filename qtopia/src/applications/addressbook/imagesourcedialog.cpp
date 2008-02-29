/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "imagesourcedialog.h"
#include <qtopia/qpeapplication.h>
#include <qtopia/imageselector.h>
#include <qtopia/image.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qpainter.h>

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
};

ImageSourceWidget::ImageSourceWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    init();
    haveImage( FALSE );
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
#ifdef Q_WS_QWS
    is->setMaximumSize(d->maxW, d->maxH);
#endif
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

    d->photoLA = new QLabel( this );
    d->photoLA->setAlignment( Qt::AlignCenter );

    QVBoxLayout *sl = new QVBoxLayout( this );
    QHBoxLayout *hl = new QHBoxLayout( sl );
    hl->addWidget( d->photoLA );

    QVBoxLayout *vl = new QVBoxLayout( hl );
    sl->addStretch();

    d->changePB = new QPushButton( tr("Pictures"), this );
    connect( d->changePB, SIGNAL(clicked()), this, SLOT(change()) );
    vl->addWidget( d->changePB );

#ifdef Q_WS_QWS
    is = new ImageServerManager(this);
    is->setMaximumSize(d->maxW, d->maxH);
    connect(is,SIGNAL(pixmapSupplied(const QPixmap&)),this,SLOT(setPixmap(const QPixmap&)));
    int n = is->serverCount();
    for (int i=0; i<n; i++) {
	ValueServiceServer s = is->server(i);
	if ( !s.isNull() ) {
	    QPushButton* pb = new QPushButton( s.pixmap(), s.name(), this ); //XXX icon too!
	    is->connectToServer(pb, SIGNAL(clicked()), i);
	    vl->addWidget(pb);
	}
	else
	qDebug("AB : server %d is null", i );
    }
#endif

    d->removePB = new QPushButton( tr("Remove"), this );
    connect( d->removePB, SIGNAL(clicked()), this, SLOT(remove()) );

    vl->addWidget( d->removePB );
    vl->addStretch();
}

#ifdef QTOPIA_DESKTOP
class ImagePreview : public QWidget, public QFilePreview
{
public:
    ImagePreview( QWidget *parent = 0, const char *name = 0 );
    void previewUrl( const QUrl &url );
};

ImagePreview::ImagePreview( QWidget *parent, const char *name )
    : QWidget( parent, name ), QFilePreview()
{
}

void ImagePreview::previewUrl( const QUrl &url )
{
    if( url.isLocalFile() )
    {
	QString fileName = url.toString( FALSE, FALSE );
	QPainter p( this );
	p.eraseRect( 0, 0, width(), height() );
	QImage img;
	if( QFile::exists( fileName ) && !(img = Image::loadScaled( fileName, width(), height()) ).isNull() )
	    p.drawImage( 0, 0, img );
	p.end();

    }
    else
    {
	qWarning("ImagePreview - Can't handle the display of remote images.");
    }
}

#endif

void ImageSourceWidget::change()
{
#ifndef QTOPIA_DESKTOP
    ImageSelectorDialog *s = new ImageSelectorDialog( this, "contactPhotoSelect", TRUE );
    if( QPEApplication::execDialog( s ) == QDialog::Accepted )
    {
	QPixmap newPix = s->selectedImage(d->maxW, d->maxH);
	if( !newPix.isNull() )
	{
	    setPixmap( newPix );
	    haveImage( TRUE );
	}
    }
    delete s;
#else
    // build a list of image formats that Qt can handle
    QStringList imgFmts = QImage::inputFormatList();
    QString formatStr = "Images (";
    for ( QStringList::Iterator it = imgFmts.begin(); it != imgFmts.end(); it++ ) {
	formatStr += QString("*.%1 ").arg((*it).lower());
    }
    formatStr = formatStr.stripWhiteSpace() + ")";
    QFileDialog *fd = new QFileDialog( QDir::homeDirPath(), formatStr, this, "imageSelector", TRUE );
    ImagePreview *ip = new ImagePreview();
    fd->setMode( QFileDialog::ExistingFile );
    fd->setContentsPreviewEnabled( TRUE );
    fd->setContentsPreview( ip, ip );
    if( fd->exec() == QDialog::Accepted && fd->selectedFile().length() )
    {
	QPixmap newPix;
        newPix.convertFromImage( Image::loadScaled( fd->selectedFile(), d->maxW, d->maxH ) );
	if( !newPix.isNull() )
	{
	    setPixmap( newPix );
	    haveImage( TRUE );
	}
    }
#endif
}

void ImageSourceWidget::setPixmap( const QPixmap &pix )
{
    d->pix = pix;
    QSize s = Image::aspectScaleSize(QMAX(pix.width(), 100), QMAX(pix.height(), 120),
		    d->photoLA->width(), d->photoLA->height());
    QImage img = pix.convertToImage();
    QPixmap pp;
    pp.convertFromImage(img.smoothScale(s.width(), s.height()));
    d->photoLA->setPixmap(pp);
    if( pix.isNull() )
    {
	haveImage( FALSE );
	QFont f = font();
	f.setItalic( TRUE );
	d->photoLA->setFont( f );
	d->photoLA->setText( tr("No Photo") );
    }
    else
    {
	haveImage( TRUE );
    }
#ifdef Q_WS_QWS
    is->setDefaultImage( d->pix.convertToImage() );
#endif
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
}

void ImageSourceWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    QPixmap pix = pixmap();
    QSize s = Image::aspectScaleSize(QMAX(pix.width(), 100), QMAX(pix.height(), 120),
		    d->photoLA->width(), d->photoLA->height());
    QImage img = pix.convertToImage();
    QPixmap pp;
    pp.convertFromImage(img.smoothScale(s.width(), s.height()));
    d->photoLA->setPixmap(pp);
}


//dialog convenience wrapper
ImageSourceDialog::ImageSourceDialog( QWidget *parent, const char *name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    init();
    
}

ImageSourceDialog::ImageSourceDialog( const QPixmap &pix, QWidget *parent, const char *name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
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
    mWidget = new ImageSourceWidget( this );
    l->addWidget( mWidget );
#ifdef QTOPIA_DESKTOP // pda has buttons in title
    QHBoxLayout *bottomLayout = new QHBoxLayout( l );
    bottomLayout->addStretch();
    QPushButton *okButton = new QPushButton( tr("OK"), this );
    okButton->setDefault( TRUE );
    connect( okButton, SIGNAL(clicked()), this, SLOT(accept()) );
    bottomLayout->addWidget( okButton );
    QPushButton *cancelButton = new QPushButton( tr("Cancel"), this );
    cancelButton->setAutoDefault( TRUE );
    connect( cancelButton, SIGNAL(clicked()), this, SLOT(reject()) );
    bottomLayout->addWidget( cancelButton );
#endif
#ifdef QTOPIA_DESKTOP
    l->setResizeMode( QLayout::FreeResize );
    QSize sh = sizeHint();
    setMinimumSize( sh );
    setMaximumSize( sh.width()*2, sh.height()*2 );
#endif
    setCaption( tr("Contact Photo") );
#if !defined(QTOPIA_DESKTOP) && !defined(QTOPIA_PHONE)
    setGeometry( qApp->desktop()->geometry() );
#endif
}

void ImageSourceDialog::setPixmap( const QPixmap &pix )
{
    mWidget->setPixmap( pix );
}

QPixmap ImageSourceDialog::pixmap() const
{
    return mWidget->pixmap();
}

