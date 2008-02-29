/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

//
// Full-screen and rotation options contributed by Robert Wittams <robert@wittams.com>
//

#include "showimg.h"
#include "settingsdialog.h"

#include <qpe/qpeapplication.h>
#include <qpe/resource.h>
#include <qpe/fileselector.h>
#include <qpe/applnk.h>
#include <qpe/qpemenubar.h>
#include <qpe/qpetoolbar.h>
#include <qpe/config.h>
#include <qwidgetstack.h>
#include <qaction.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qtimer.h>

//===========================================================================
/*
  Contains the scaled image area and the status label
*/

ImagePane::ImagePane( QWidget *parent=0 ) : QWidget( parent ), vb(0)
{
    image = new ImageWidget( this );
    connect( image, SIGNAL( clicked() ), this, SIGNAL( clicked() ) );
    status = new QLabel( this );
    status->setFixedHeight( fontMetrics().height() + 4 );
    showStatus();
    imageWidth = imageHeight = 0;
}

void ImagePane::setPixmap( const QPixmap &pm )
{
    image->setPixmap( pm );
    image->repaint( false );
    imageWidth = pm.width();
    imageHeight = pm.height();
}

void ImagePane::showStatus()
{
    delete vb;
    vb = new QVBoxLayout( this );
    vb->addWidget( image );
    status->show();
    vb->addWidget( status );
}

void ImagePane::hideStatus()
{
    delete vb;
    vb = new QVBoxLayout( this );
    vb->addWidget( image );
    status->hide();
}

void ImagePane::keyPressEvent(QKeyEvent *e)
{
    emit keypress(e->key());
}

void ImagePane::closeEvent(QCloseEvent *e)
{
    e->ignore();
}

//===========================================================================
/*
  Draws the portion of the scaled pixmap that needs to be updated
*/

void ImageWidget::paintEvent( QPaintEvent *e )
{
    QPainter painter(this);

    painter.setClipRect(e->rect());
    painter.setBrush( black );
    painter.drawRect( 0, 0, width(), height() );

    if ( pixmap.size() != QSize( 0, 0 ) ) { // is an image loaded?
	painter.drawPixmap((width() - pixmap.width()) / 2,
	(height() - pixmap.height()) / 2, pixmap);
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *)
{
    emit clicked();
}

//===========================================================================
/*
 The main window with toolbars and fileselector. The image pane is shown inside it.
*/

ImageViewer::ImageViewer( QWidget *parent, const char *name, int wFlags )
    : QMainWindow( parent, name, wFlags ), filename( 0 ), 
      bFromDocView( FALSE )
{
    setCaption( tr("Image Viewer") );
    setIcon( Resource::loadPixmap( "ImageViewer" ) );
    setBackgroundMode( PaletteButton );

    needPmScaled0 = TRUE;
    needPmScaled90 = TRUE;
    rotated90 = FALSE;
    isFullScreen = FALSE;

    setToolBarsMovable( FALSE );

    toolBar = new QPEToolBar( this );
    toolBar->setHorizontalStretchable( TRUE );

    menubar = new QPEMenuBar( toolBar );

    stack = new QWidgetStack( this );
    stack->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setCentralWidget( stack );

    imagePanel = new ImagePane( stack );
    connect(imagePanel, SIGNAL(clicked()), this, SLOT(toggleFullscreen()));
    connect(imagePanel, SIGNAL(keypress(int)), this, SLOT(handleKeypress(int)));

    connect(this, SIGNAL(keypress(int)), this, SLOT(handleKeypress(int)));

    fileSelector = new FileSelector("image/*", stack, "fs", FALSE, FALSE);
    connect( fileSelector, SIGNAL( closeMe() ), this, SLOT( closeFileSelector() ) );
    connect( fileSelector, SIGNAL( fileSelected( const DocLnk &) ), this, SLOT( openFile( const DocLnk & ) ) );
    imageList = fileSelector->fileList();

    QPopupMenu *edit = new QPopupMenu( menubar );
    QPopupMenu *view = new QPopupMenu( menubar );

    menubar->insertItem( "Image", edit );
    menubar->insertItem( "View", view );

    toolBar = new QPEToolBar( this );

    openAction = new QAction( tr( "Open" ), Resource::loadIconSet( "fileopen" ), QString::null, 0, this, 0 );
    connect( openAction, SIGNAL( activated() ), this, SLOT( open() ) );
    openAction->addTo( toolBar );
    openAction->addTo( edit );

    edit->insertSeparator();
    edit->insertItem(tr("Horizontal flip"), this, SLOT(hFlip()), 0);
    edit->insertItem(tr("Vertical flip"), this, SLOT(vFlip()), 0);

    QAction *a;
    a = new QAction( tr( "Rotate 180" ), Resource::loadIconSet( "repeat" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( rot180() ) );
    a->addTo( toolBar );
    a->addTo( edit );

    a = new QAction( tr( "Rotate 90"), Resource::loadIconSet( "rotate90" ), QString::null, 0, this, 0);
    connect( a, SIGNAL( activated() ), this, SLOT( rot90() ) );
    a->addTo( toolBar );
    a->addTo( edit );

    a = new QAction( tr( "Fullscreen" ), Resource::loadIconSet( "fullscreen" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fullScreen() ) );
    a->addTo( toolBar );
    a->addTo( view);

    slideAction = new QAction( tr( "Slide show" ), Resource::loadIconSet( "slideshow" ), QString::null, 0, this, 0 );
    slideAction->setToggleAction( TRUE );
    connect( slideAction, SIGNAL( toggled(bool) ), this, SLOT( slideShow(bool) ) );
    slideAction->addTo( view);
    slideAction->addTo( toolBar );
    slideAction->setEnabled(imageList.count() != 0);

    prevImageAction = new QAction(tr("Previous"), Resource::loadIconSet("back"),
	QString::null, 0, this, 0);
    connect(prevImageAction, SIGNAL(activated()), this, SLOT(prevImage()));
    prevImageAction->addTo(toolBar);
    prevImageAction->addTo(view);
    prevImageAction->setEnabled(imageList.count() != 0);

    nextImageAction = new QAction(tr("Next"), Resource::loadIconSet("forward"),
	QString::null, 0, this, 0);
    connect(nextImageAction, SIGNAL(activated()), this, SLOT(nextImage()));
    nextImageAction->addTo(toolBar);
    nextImageAction->addTo(view);
    nextImageAction->setEnabled(imageList.count() != 0);

    view->insertSeparator();
    view->insertItem(tr("Settings..."), this, SLOT(settings()), 0);

    stack->raiseWidget( fileSelector );
    openAction->setEnabled( FALSE );

    setMouseTracking( TRUE );
    slideTimer = new QTimer( this );
    connect( slideTimer, SIGNAL(timeout()), this, SLOT(slideUpdate()) );

    Config config( "ImageViewer" );
    config.setGroup( "SlideShow" );
    slideDelay = config.readNumEntry( "Delay", 5 );
    slideRepeat = config.readBoolEntry( "Repeat", FALSE );
    slideReverse = config.readBoolEntry("Reverse", FALSE);

    config.setGroup("Default");
    rotateOnLoad = config.readBoolEntry("Rotate", FALSE);
    fastLoad = config.readBoolEntry("FastLoad", TRUE);
}

ImageViewer::~ImageViewer()
{
    if ( isFullScreen)
	delete imagePanel; // when fullScreen, it is reparented off the QWidgetStack
}

void ImageViewer::settings()
{
    SettingsDialog dlg( this, 0, TRUE );
    dlg.setDelay( slideDelay );
    dlg.setRepeat( slideRepeat );
    dlg.setReverse( slideReverse );
    dlg.setRotate(rotateOnLoad);
    dlg.setFastLoad(fastLoad);

    if ( QPEApplication::execDialog(&dlg) == QDialog::Accepted ) {
	slideDelay = dlg.delay();
	slideRepeat = dlg.repeat();
	slideReverse = dlg.reverse();
	rotateOnLoad = dlg.rotate();
	fastLoad = dlg.fastLoad();

	Config config( "ImageViewer" );
	config.setGroup( "SlideShow" );
	config.writeEntry( "Delay", slideDelay );
	config.writeEntry( "Repeat", slideRepeat );
	config.writeEntry("Reverse", slideReverse);

	config.setGroup("Default");
	config.writeEntry("Rotate", rotateOnLoad);
	config.writeEntry("FastLoad", fastLoad);
    }
}

void ImageViewer::setDocument(const QString& fileref)
{
    bFromDocView = TRUE;
    DocLnk link( fileref );
    if ( link.isValid() )
	openFile( link.name(), link.file() );
    else
	openFile( fileref, fileref );
}

void ImageViewer::show()
{
    normalView();
    QMainWindow::show();
}

void ImageViewer::openFile( const DocLnk &file )
{
    openFile( file.name(), file.file() );
}

void ImageViewer::openFile( const QString &name, const QString &file )
{
    closeFileSelector();
    updateCaption( name );
    loadFilename( file );

    if (slideTimer->isActive()) {
	slideTimer->start(slideDelay * 1000, FALSE);
    }

}

void ImageViewer::open()
{
    slideAction->setOn( FALSE );
    stack->raiseWidget(fileSelector);
    openAction->setEnabled( FALSE );
    updateCaption();
}

void ImageViewer::closeFileSelector()
{
    stack->raiseWidget(imagePanel);
    openAction->setEnabled( TRUE );
}

void ImageViewer::updateCaption( QString name )
{
    if ( name.isEmpty() || stack->visibleWidget() == fileSelector ) {
	setCaption( tr("Image Viewer") );
    } else {
	int sep = name.findRev( '/' );
	if ( sep >= 0 )
	    name = name.mid( sep+1 );
	setCaption( name + tr(" - Image Viewer") );
    }
}

void ImageViewer::loadFilename( const QString &file ) {
    if ( file && file != filename ) {
	filename = file;
	imagePanel->statusLabel()->setText( tr("Loading image...") );
	qApp->processEvents();

	QString	param;
	QImageIO iio;

	iio.setFileName(filename);

	iio.setParameters("GetHeaderInformation");
	iio.read();
	imagewidth = iio.image().width();
	imageheight = iio.image().height();
	if ( !iio.image().bits() ) {
	    // GetHeaderInformation is supported
	    int maxsize = QMAX(qApp->desktop()->width(),
		qApp->desktop()->height());

	    //
	    // Scale the image.  If we're loading fast, use the
	    // shrink parameter to help out.
	    //
	    param.sprintf("Scale( %i, %i, %s ), %s, Shrink( %i )",
		maxsize, maxsize, "ScaleMin",
		fastLoad ? ", Fast" : "",
		fastLoad ? QMAX(imagewidth/maxsize, imageheight/maxsize) : 1);

	    iio.setParameters(param);
	    iio.read();
	}
	image = iio.image();

	matrix.reset();
	rotated90 = FALSE;

	if (rotateOnLoad) {
	    rotated90 = TRUE;
	    matrix.rotate( -90.0 );
	}
	scale( TRUE );
    } else {
	imagePanel->setPixmap( scaledPixmap( FALSE ) );
    }
}

bool ImageViewer::loadSelected()
{
    if ( stack->visibleWidget() == fileSelector ) {
	DocLnk link = fileSelector->selectedDocument();
	if ( link.fileKnown() ) {
	    openFile( link.name(), link.file() );
	    filename = link.file();
	    return true;
	}
    }
    if ( !image.isNull() ) {
	closeFileSelector();
	updateCaption( filename );
	return true;
    }
    return false;
}

int ImageViewer::h()
{
    if ( !isFullScreen)
	 return  height() - menubar->heightForWidth( width() )
		    - imagePanel->statusLabel()->height();
    else
         return qApp->desktop()->height();
}


const QPixmap &ImageViewer::scaledPixmap( bool newImage )
{
    Q_UNUSED( newImage );
    if ( rotated90 ) {
	if ( needPmScaled90 ) {
	    needPmScaled90 = FALSE;
	    // Fit the image rotated 90 to the size of the widget
	    // Only scale if it is a new image, or the width and the
	    // height of the widget changes
	    double r = QMIN((double)h()/image.width(),
		(double)width()/image.height());
	    int w = int(image.width() * r);
	    int h = int(image.height() * r);

	    if ( !pmScaled90.convertFromImage( image.smoothScale( w, h ) ) )
		pmScaled90.resize( 0, 0 );
	    // qDebug("doing scale 90");
	}
	return pmScaled = pmScaled90.xForm( matrix );
    } else {
	if ( needPmScaled0 ) {
	    needPmScaled0 = FALSE;
	    // Fit the image to the size of the widget
	    double r = QMIN((double)h()/image.height(),
		(double)width()/image.width());
	    int w = int(image.width() * r);
	    int h = int(image.height() * r);
	    if ( !pmScaled0.convertFromImage( image.smoothScale( w, h ) ) )
		pmScaled0.resize( 0, 0 );
	    // qDebug("doing scale 0");
	}
	return pmScaled = pmScaled0.xForm( matrix );
    }
}


/*
  This functions scales the image to fit the widget size
*/
void ImageViewer::scale( bool newImage )
{
    if ( !image.isNull() ) {
	needPmScaled0 = TRUE;
	needPmScaled90 = TRUE;
	imagePanel->setPixmap( scaledPixmap( newImage ) );
	updateStatus();
    }
}

/*
  If a valid image was loaded it will be scaled to fit the new widget size
*/
void ImageViewer::resizeEvent( QResizeEvent * )
{
    QLabel *status = imagePanel->statusLabel();
    status->setGeometry( 0, height() - status->height(), width(),
	status->height() );

    if ( image.isNull() )		// there is no image loaded
	return;

    if ( image.hasAlphaBuffer() )
	erase();

    // if different size, scale pmScaled to new widget size
    if ( width() != imagePanel->w() || h() != imagePanel->h() )
	scale( FALSE );
}

void ImageViewer::setScaledImage()
{
    if ( loadSelected() ) {
	imagePanel->setPixmap( scaledPixmap( FALSE ) );
	updateStatus();
    }
}

void ImageViewer::hFlip()
{
    matrix.scale( -1.0, 1.0 );
    setScaledImage();
}

void ImageViewer::vFlip()
{
    matrix.scale( 1.0, -1.0 );
    setScaledImage();
}

void ImageViewer::rot180()
{
    matrix.rotate( 180.0 );
    setScaledImage();
}

void ImageViewer::rot90()
{
    rotated90 = !rotated90;
    matrix.rotate( -90.0 );
    setScaledImage();
}

void ImageViewer::rot270()
{
    rotated90 = !rotated90;
    matrix.rotate(90.0);
    setScaledImage();
}

void ImageViewer::toggleFullscreen()
{
    if ( isFullScreen )
	normalView();
    else
	fullScreen();
}

void ImageViewer::normalView()
{
    if ( !imagePanel->parentWidget() ) {
	isFullScreen = FALSE;
	stack->addWidget( imagePanel, 1 );
	scale( FALSE );
	imagePanel->showStatus();
	stack->raiseWidget( imagePanel );
    }
}

void ImageViewer::fullScreen()
{
    // Full-screen and rotation options
    // contributed by Robert Wittams <robert@wittams.com>
    if ( imagePanel->parentWidget() && loadSelected() ) {
	isFullScreen = TRUE;
	imagePanel->reparent(0,QPoint(0,0));
	imagePanel->resize(qApp->desktop()->width(), qApp->desktop()->height());
	imagePanel->hideStatus();
	scale( FALSE );
	imagePanel->showFullScreen();
    }
}

void ImageViewer::slideShow( bool on )
{
    if (on) {
	if (!imageList.isEmpty()) {
	    slideTimer->start(slideDelay * 1000, FALSE);
	    filename = "";		// force restart
	    slideReverse ? prevImage() : nextImage();
	}
    } else {
	slideTimer->stop();
    }
}

void ImageViewer::slideUpdate()
{
    bool final_image = slideReverse ? prevImage() : nextImage();

    if (final_image && !slideRepeat) {
	slideTimer->stop();
	slideAction->setOn(FALSE);
    }
}

void ImageViewer::updateStatus()
{
    if ( image.isNull() ) {
	if ( filename )
	    imagePanel->statusLabel()->setText( tr("Could not load image") );
	else
	    imagePanel->statusLabel()->setText( tr("No image - select Open "
		"from File menu.") );
    } else {
	QString message("%1x%2");
	message = message.arg(imagewidth).arg(imageheight);
	if ( imagewidth != imagePanel->w() || imageheight != imagePanel->h() )
	    message += QString(" [%1x%2]").arg(imagePanel->w()).arg(imagePanel->h());
	if ( image.numColors() > 0 ) {
	    message += tr(", %1 colors").arg(image.numColors());
	} else if ( image.depth() >= 16 ) {
	    message += tr(" True color");
	}
	if ( image.hasAlphaBuffer() ) {
	    if ( image.depth() == 8 ) {
		int i;
		bool alpha[256];
		int nalpha=0;

		for (i=0; i<256; i++)
		    alpha[i] = FALSE;

		for (i=0; i<image.numColors(); i++) {
		    int alevel = image.color(i) >> 24;
		    if (!alpha[alevel]) {
			alpha[alevel] = TRUE;
			nalpha++;
		    }
		}
		message += tr(", %1 alpha levels").arg(nalpha);
	    } else {
		// Too many pixels to bother counting.
		message += tr(", 8-bit alpha channel");
	    }
	}
	imagePanel->statusLabel()->setText(message);
    }
}

void ImageViewer::closeEvent( QCloseEvent *e )
{
    if ( stack->visibleWidget() == imagePanel && !bFromDocView ) {
	e->ignore();
	open();
	updateCaption();
    } else {
	bFromDocView = FALSE;
	e->accept();
    }
}

//
// Return the index into the imageList of the currently viewed
// image (ie. ImageViewer::filename in ImageViewer::imageList).
//
int
ImageViewer::imageIndex(void)
{
    QValueListConstIterator<DocLnk> i;
    int	index;

    if (imageList.count() == 0) {
	return -1;
    }

    for (index = 0, i = imageList.begin(); i != imageList.end(); ++i, index++) {
	if ((*i).file() == filename) {
	    return index;
	}
    }

    return -1;
}

//
// Display the image after the current one in the image list.
// Return TRUE if the next call to nextImage() will wrap around to the
// first image in the list (ie. we're now viewing the last image in the list).
//
bool
ImageViewer::nextImage(void)
{
    int	idx = 0;

    if (imageList.count() > 0) {
	idx = imageIndex();
	if (idx != -1) {
	    if (idx == int(imageList.count() - 1)) {
		idx = 0;
	    } else {
		idx++;
	    }
	} else {
	    idx = 0;
	}
	openFile(imageList[idx]);
    }

    return idx == int(imageList.count() - 1) ? TRUE : FALSE;
}

//
// Display the image preceeding the current one in the image list.
// Return TRUE if the next call to prevImage() will wrap around to the last
// image in the list (ie. we're now viewing the first image in the list).
//
bool
ImageViewer::prevImage(void)
{
    int idx = -1;

    if (imageList.count() > 0) {
	idx = imageIndex();
	if (idx != -1) {
	    if (idx == 0) {
		idx = imageList.count() - 1;
	    } else {
		idx--;
	    }
	} else {
	    idx = imageList.count() - 1;
	}
	openFile(imageList[idx]);
    }

    return idx == 0 ? TRUE : FALSE;
}

void
ImageViewer::keyPressEvent(QKeyEvent *e)
{
    emit keypress(e->key());
}

void
ImageViewer::handleKeypress(int keycode)
{
    switch (keycode) {
    case Qt::Key_Right:
	nextImage();
	break;

    case Qt::Key_Left:
	prevImage();
	break;

    case Qt::Key_Up:
	rot270();
	break;

    case Qt::Key_Down:
	rot90();
	break;

    case Qt::Key_Escape:
	if (isFullScreen) {
	    normalView();
	}
	break;
    }
}
