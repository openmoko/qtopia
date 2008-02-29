/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

//
// Full-screen and rotation options contributed by Robert Wittams <robert@wittams.com>
//

#include "showimg.h"
#include "settingsdialog.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/storage.h>
#include <qtopia/fileselector.h>
#include <qtopia/applnk.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qwidgetstack.h>
#include <qaction.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qcopchannel_qws.h>
#include <qtopia/docproperties.h>
#ifdef QTOPIA_PHONE
# include <qtopia/contextmenu.h>
# include <qtopia/contextbar.h>
#endif

//===========================================================================
/*
  Contains the scaled image area and the status label
*/

ImagePane::ImagePane( QWidget *parent ) : QWidget( parent ), vb(0)
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

void ImagePane::showBusy()
{
    image->showBusy();
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

#ifdef QTOPIA_PHONE
void ImagePane::keyPressEvent(QKeyEvent *e)
{
    if( !Global::mousePreferred() ) {
	QWidget::keyPressEvent(e);
	if (!e->isAccepted()) {
	    e->accept();
	    emit keypress(e->key());
	}
    }
}
#endif

void ImagePane::closeEvent(QCloseEvent *e)
{
    e->ignore();
}

ImageWidget::ImageWidget(QWidget *parent) :
    QWidget( parent ), pixmap( 0 )
{
    QIconSet waitset = Resource::loadIconSet("wait");

    wait = waitset.pixmap();
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

//    QTOPIA_PROFILE("drawn image");
}

/*
  Shows a busy indicator
 */
void ImageWidget::showBusy()
{
    QPainter painter(this);
    painter.setBrush( white );
    int pw = wait.width();
    int ph = wait.height();
    int w = pw * 3/2;
    int h = ph * 3/2;
    painter.drawEllipse( 0, 0, w, h );
    painter.drawPixmap( (w-pw)/2+1, (h-ph)/2+1, wait );
    
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
    : QMainWindow( parent, name, wFlags | Qt::WResizeNoErase ),
      filename( 0 ), 
      doc(NULL),
      bFromDocView( FALSE ),
      edit(0)
{
    setCaption( tr("Image Viewer") );
    setIcon( Resource::loadPixmap( "ImageViewer" ) );
    setBackgroundMode( PaletteButton );

    needPmScaled0 = TRUE;
    needPmScaled90 = TRUE;
    rotated90 = FALSE;
    isFullScreen = FALSE;

    stack = new QWidgetStack( this );
    stack->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setCentralWidget( stack );

    imagePanel = new ImagePane( stack );
    connect(imagePanel, SIGNAL(clicked()), this, SLOT(toggleFullscreen()));
    connect(imagePanel, SIGNAL(keypress(int)), this, SLOT(handleKeypress(int)));

    connect(this, SIGNAL(keypress(int)), this, SLOT(handleKeypress(int)));

    fileSelector = new FileSelector("image/" "*", stack, "fs", FALSE, FALSE);
    connect( fileSelector, SIGNAL( closeMe() ), this, SLOT( closeFileSelector() ) );
    connect( fileSelector, SIGNAL( fileSelected(const DocLnk&) ), this, SLOT( openFile(const DocLnk&) ) );
    connect(fileSelector, SIGNAL(categoryChanged()), this, SLOT(docsChanged()));
    connect(fileSelector, SIGNAL(typeChanged()), this, SLOT(docsChanged()));

    openAction = new QAction( tr( "Open" ), Resource::loadIconSet( "fileopen" ), QString::null, 0, this, 0 );
    connect( openAction, SIGNAL( activated() ), this, SLOT( open() ) );

    flipAction = new QAction( tr( "Rotate 180" ), Resource::loadIconSet( "repeat" ), QString::null, 0, this, 0 );
    connect( flipAction, SIGNAL( activated() ), this, SLOT( rot180() ) );

    rotateAction = new QAction( tr( "Rotate 90"), Resource::loadIconSet( "rotate90" ), QString::null, 0, this, 0);
    connect( rotateAction, SIGNAL( activated() ), this, SLOT( rot90() ) );

    propAction = new QAction(tr("Properties..."), QIconSet(), QString::null, 0, this, 0);
    connect(propAction, SIGNAL( activated() ), this, SLOT(properties()));

    fullscreenAction = new QAction( tr( "Fullscreen" ), Resource::loadIconSet( "fullscreen" ), QString::null, 0, this, 0 );
    connect( fullscreenAction, SIGNAL( activated() ), this, SLOT( fullScreen() ) );

    slideAction = new QAction( tr( "Slide show" ), Resource::loadIconSet( "slideshow" ), QString::null, 0, this, 0 );
    slideAction->setToggleAction( TRUE );
    connect( slideAction, SIGNAL( toggled(bool) ), this, SLOT( slideShow(bool) ) );
    slideAction->setEnabled( FALSE );

    prevImageAction = new QAction(tr("Previous"), Resource::loadIconSet("back"),
	QString::null, 0, this, 0);
    connect(prevImageAction, SIGNAL(activated()), this, SLOT(prevImage()));

    nextImageAction = new QAction(tr("Next"), Resource::loadIconSet("forward"),
	QString::null, 0, this, 0);
    connect(nextImageAction, SIGNAL(activated()), this, SLOT(nextImage()));

#ifndef QTOPIA_PHONE
    setToolBarsMovable( FALSE );

    toolBar = new QPEToolBar( this );
    toolBar->setHorizontalStretchable( TRUE );
    menubar = new QPEMenuBar( toolBar );

    edit = new QPopupMenu( menubar );
    QPopupMenu *view = new QPopupMenu( menubar );

    menubar->insertItem( tr("Image"), edit );
    menubar->insertItem( tr("View"), view );

    toolBar = new QPEToolBar( this );

    openAction->addTo( toolBar );
    openAction->addTo( edit );

    edit->insertSeparator();
    hflip_id = edit->insertItem(tr("Horizontal flip"), this, SLOT(hFlip()), 0);
    vflip_id = edit->insertItem(tr("Vertical flip"), this, SLOT(vFlip()), 0);

    flipAction->addTo( toolBar );
    flipAction->addTo( edit );
    rotateAction->addTo( toolBar );
    rotateAction->addTo( edit );
    edit->insertSeparator();
    propAction->addTo(edit);
    fullscreenAction->addTo( toolBar );
    fullscreenAction->addTo( view);
    slideAction->addTo( view);
    slideAction->addTo( toolBar );
    prevImageAction->addTo(toolBar);
    prevImageAction->addTo(view);
    nextImageAction->addTo(toolBar);
    nextImageAction->addTo(view);

    view->insertSeparator();
    view->insertItem(tr("Settings..."), this, SLOT(settings()), 0);
#else
    contextMenu = new ContextMenu(this);
    fullscreenAction->addTo(contextMenu);
    slideAction->addTo(contextMenu);
    contextMenu->insertItem(tr("Settings..."), this, SLOT(settings()), 0);
    ContextBar::setLabel(this, Key_Select, ContextBar::NoLabel);
#endif

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
    rot = RotationDirection(config.readNumEntry("Rotate"));
    fastLoad = config.readBoolEntry("FastLoad", TRUE);
    smallScale = config.readBoolEntry("SmallScale", FALSE);

    storage = new StorageInfo( this );
    connect(storage, SIGNAL(disksChanged()), this, SLOT(updateDocs()));
    connect(qApp, SIGNAL(linkChanged(const QString&)), this, SLOT(updateDocs()));

    setControls();
    updateDocs();
}

void ImageViewer::updateDocs()
{
    QTimer::singleShot(0, this, SLOT(docsChanged()));
}

ImageViewer::~ImageViewer()
{
    if (doc != NULL) {
	delete doc;
    }

    if ( isFullScreen)
	delete imagePanel; // when fullScreen, it is reparented off the QWidgetStack
}

//
// Popup image properties dialog.
//
void
ImageViewer::properties(void)
{
    if (stack->visibleWidget() == fileSelector) {
	if (doc == NULL) {
	    doc = new DocLnk();
	}
	*doc = fileSelector->selectedDocument();
    }

    if (doc) {
	// pause slideshow if on
	bool on = slideTimer->isActive();
	if (on)
	    slideTimer->stop();

	DocPropertiesDialog *dp = new DocPropertiesDialog(doc, this);
	QPEApplication::execDialog( dp );
	delete dp;
	// if link is changed in the properties dialog,
	// we get a linkChanged message to handle

	if (on)
	    slideTimer->start(slideDelay * 1000, FALSE);
    }
}

void ImageViewer::docsChanged(void)
{
    // Avoid causing the fileSelector to scan until needed. It's okay if
    // we have already caused it to scan in which case imageList is not
    // empty, or we need it to scan because we are viewing the file selector.
    if ( fileSelector->isVisible() || !imageList.isEmpty() ) {
	imageList.clear();
	fileSelector->reread();
	imageList = fileSelector->fileList();
	setControls();
    }
}

void ImageViewer::settings()
{
    SettingsDialog dlg( this, 0, TRUE );
    dlg.setDelay( slideDelay );
    dlg.setRepeat( slideRepeat );
    dlg.setReverse( slideReverse );
    dlg.setRotation(rot);
    dlg.setFastLoad(fastLoad);
    dlg.setSmallScale(smallScale);

    if ( QPEApplication::execDialog(&dlg) == QDialog::Accepted ) {
	slideDelay = dlg.delay();
	slideRepeat = dlg.repeat();
	slideReverse = dlg.reverse();
	rot = dlg.rotation();
	fastLoad = dlg.fastLoad();
	smallScale = dlg.smallScale();

	Config config( "ImageViewer" );
	config.setGroup( "SlideShow" );
	config.writeEntry( "Delay", slideDelay );
	config.writeEntry( "Repeat", slideRepeat );
	config.writeEntry("Reverse", slideReverse);

	config.setGroup("Default");
	config.writeEntry("Rotate", rot);
	config.writeEntry("FastLoad", fastLoad);
	config.writeEntry("SmallScale", smallScale);
    }

    //
    // Reload current image in case viewing options have changed.
    //
    if (stack->visibleWidget() != fileSelector) {
	loadFilename(filename);
    }
}

void ImageViewer::setDocument(const QString& fileref)
{
    bFromDocView = TRUE;
    DocLnk link( fileref );

    delete doc;
    doc = new DocLnk(fileref);

    if ( link.isValid() )
	openFile( link.name(), link.file() );
    else
	openFile( fileref, fileref );
}

void ImageViewer::show()
{
    QMainWindow::show();
    normalView();
}

void ImageViewer::openFile( const DocLnk &file )
{
    delete doc;
    doc = new DocLnk(file);

    openFile( file.name(), file.file() );
}

void ImageViewer::openFile( const QString &name, const QString &file )
{
    if (stack->visibleWidget() == fileSelector) {
	imagePanel->setPixmap(QPixmap());
    }

    closeFileSelector();
    updateCaption( name );
    loadFilename( file );
    setControls();

    if (slideTimer->isActive()) {
	slideTimer->start(slideDelay * 1000, FALSE);
    }

}

void ImageViewer::open()
{
    slideAction->setOn( FALSE );
    stack->raiseWidget(fileSelector);
    fileSelector->setFocus();
    openAction->setEnabled( FALSE );
    updateCaption();
    updateDocs();
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

void ImageViewer::loadFilename( const QString &file )
{
    if ( file ) {
	filename = file;
	if ( !slideTimer->isActive() )
	    imagePanel->showBusy();

	imagePanel->statusLabel()->setText( tr("Loading image...") );
	qApp->processEvents();

	QString	param;
	QImageIO iio;

	iio.setFileName(filename);

	iio.setParameters("GetHeaderInformation");
	if (iio.read() == FALSE) {
	    image.reset();
	    imagePanel->statusLabel()->setText(tr("Image load failed"));
	    imagePanel->setPixmap(QPixmap());

	    return;
	}

	imagewidth = iio.image().width();
	imageheight = iio.image().height();
	if ( !iio.image().bits() ) {
	    // GetHeaderInformation is supported

	    //
	    // Don't scale if we don't have to.
	    //
	    if (!smallScale &&
		    imagewidth < qApp->desktop()->width() &&
		    imageheight < qApp->desktop()->height()) {
		param.sprintf("%s", fastLoad ? "Fast" : ""); // No tr
	    } else {
		int maxsize = QMAX(qApp->desktop()->width(),
		    qApp->desktop()->height());

		//
		// Scale the image.  If we're loading fast, use the
		// shrink parameter to help out.
		//
		param.sprintf("Scale( %i, %i, %s ), %s, Shrink( %i )", // No tr
		    maxsize, maxsize, "ScaleMin",
		    fastLoad ? ", Fast" : "", // No tr
		    fastLoad ? QMAX(imagewidth/maxsize, imageheight/maxsize):1);

	    }
	    iio.setParameters(param);
	    iio.read();
	}
	image = iio.image();

	matrix.reset();
	rotated90 = FALSE;

	//
	// Don't rotate if the image is square.
	//
	if (imageheight != imagewidth) {
	    bool portraitDisplay = imagePanel->height() > imagePanel->width();
	    bool portraitImage = imageheight > imagewidth;
	
	    if (rot != rotate_none && portraitImage != portraitDisplay ) {
		rotated90 = TRUE;
		if ( rot == rotate_clockwise )
		    matrix.rotate( 90.0 );
		else
		    matrix.rotate( -90.0 );
	    }
	}
	scale( TRUE );
    } else {
	image.reset();
    }
}

void ImageViewer::setControls()
{
    bool validPicture = !image.isNull() && stack->visibleWidget() == imagePanel;
    propAction->setEnabled(validPicture);
    rotateAction->setEnabled(validPicture);
    flipAction->setEnabled(validPicture);
    fullscreenAction->setEnabled(validPicture);

    bool multipleImages = imageList.count() > 1;
    slideAction->setEnabled(multipleImages);
    prevImageAction->setEnabled(validPicture && multipleImages);
    nextImageAction->setEnabled(validPicture && multipleImages);

    if (edit) {
	edit->setItemEnabled(hflip_id, validPicture);
	edit->setItemEnabled(vflip_id, validPicture);
    }
}

bool ImageViewer::loadSelected()
{
    if ( stack->visibleWidget() == fileSelector ) {
	DocLnk link = fileSelector->selectedDocument();
	if ( link.fileKnown() ) {
	    delete doc;
	    doc = new DocLnk(link);

	    openFile( link.name(), link.file() );
	    filename = link.file();
	    return true;
	}
    }
    if ( !image.isNull() ) {
	closeFileSelector();
	return true;
    }
    return false;
}

int ImageViewer::h()
{
    if ( !isFullScreen)
#ifndef QTOPIA_PHONE
	 return  height() - menubar->heightForWidth( width() )
		    - imagePanel->statusLabel()->height();
#else
	 return  height() - imagePanel->statusLabel()->height();
#endif
    else
         return qApp->desktop()->height();
}


const QPixmap &ImageViewer::scaledPixmap( bool newImage )
{
    Q_UNUSED( newImage );

    int	sw = 0;
    int	sh = 0;

    if (!image.isNull()) {
	//
	// Leaves dimensions as they are if we're not scaling.
	//
	if (!smallScale &&
		image.width() < imagePanel->width() &&
		image.height() < h()) {
	    sw = image.width();
	    sh = image.height();
	} else {
	    //
	    // Setup scaling first.
	    //
	    sw = rotated90 ? h() : imagePanel->width();
	    sh = rotated90 ? imagePanel->width() : h();

	    int t1 = image.width() * sh;
	    int t2 = image.height() * sw;

	    if (t1 > t2) {
		sh = t2 / image.width();
	    } else {
		sw = t1 / image.height();
	    }
	}
    }

    //
    // Scale (and rotate, if required).
    //
    if ( rotated90 ) {
	if ( needPmScaled90 ) {
	    needPmScaled90 = FALSE;
	    if ( !pmScaled90.convertFromImage( image.smoothScale( sw, sh ) ) )
		pmScaled90.resize( 0, 0 );
	}
	return pmScaled = pmScaled90.xForm( matrix );
    } else {
	if ( needPmScaled0 ) {
	    needPmScaled0 = FALSE;
	    if ( !pmScaled0.convertFromImage( image.smoothScale( sw, sh ) ) )
		pmScaled0.resize( 0, 0 );
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
    if ( bFromDocView || !imagePanel->parentWidget() ) {
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

#ifdef QT_KEYPAD_MODE
void
ImageViewer::keyPressEvent(QKeyEvent *e)
{
    QMainWindow::keyPressEvent(e);
    if (!e->isAccepted()) {
	if ((e->key() != Key_Back && e->key() != Key_No) || stack->visibleWidget() != fileSelector) {
	    e->accept();
	    emit keypress(e->key());
	}
    }
}
#endif

void
ImageViewer::handleKeypress(int keycode)
{
    if ( stack->visibleWidget() == fileSelector )
	return;

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
#ifdef QT_KEYPAD_MODE
    case Qt::Key_Select:
#endif	// QT_KEYPAD_MODE
	if (isFullScreen) {
	    normalView();
	}
	break;


#ifdef QT_KEYPAD_MODE
    case Qt::Key_No:
    case Qt::Key_Back:
#endif
    case Qt::Key_Space:
	if (isFullScreen) {
	    normalView();
	} else {
	    if (bFromDocView) {
		close();
	    } else {
		open();
	    }
	}
	break;
    }
}

