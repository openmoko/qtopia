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

#include "photoeditui.h"

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qdocumentproperties.h>
#include <qtopiaipcenvelope.h>
#include <qtopiasendvia.h>
#include <qtopiaservices.h>
#include <qcontent.h>
#include <qimagedocumentselector.h>
#include <qdrmcontentplugin.h>

#include <QPoint>
#include <QMessageBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSize>
#include <QCloseEvent>
#include <QDSActionRequest>
#include <QDSData>
#include <QPixmap>
#include <QMimeType>
#include <QStackedLayout>
#include <QMenu>

#include <cmath>

PhotoEditUI::PhotoEditUI( QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f )
    , ui_state( SELECTOR )
    , editor_state( VIEW )
    , only_editor( false )
    , service_requested( false )
    , was_fullscreen( false )
    , edit_canceled( false )
    , editor_state_changed( false )
    , selector_image( QDrmRights::Display )
    , separator_action(0)
    , properties_action(0)
    , beam_action(0)
    , print_action(0)
    , delete_action(0)
    , edit_action(0)
    , slide_show_action(0)
    , selector_menu(0)
    , image_selector(0)
    , region_selector(0)
    , navigator(0)
    , brightness_slider(0)
    , zoom_slider(0)
    , image_ui(0)
    , image_processor(0)
    , image_io(0)
    , slide_show_dialog(0)
    , slide_show_ui(0)
    , slide_show(0)
    , widget_stack(0)
    , currEditImageRequest(0)
{
    setWindowTitle( tr( "Pictures" ) );

    QDrmContentPlugin::initialize();

    widget_stack = new QStackedLayout( this );

    // Respond to service requests
    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
        this, SLOT(appMessage(QString,QByteArray)) );

    new PhotoEditService( this );

    // Respond to file changes
    connect( qApp,
        SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
        this,
        SLOT(contentChanged(QContentIdList,QContent::ChangeType)));

    // The rest of the UI will be created by calling init(bool) from resizeEvent(...), because
    // by that time we will know whether we are creating PhotoEditUI for a single document or for
    // a list of thumbnails.
}

PhotoEditUI::~PhotoEditUI()
{
    delete currEditImageRequest;
    currEditImageRequest = 0;
}

// This finishes constructing the UI. We figure out whether or not to call this by looking at
// whether image_io has been constructed.
// ****HACK HACK HACK*** (again). We're working on the assumption that setDocument(...) only
// gets called when you're not constructing a list, i.e. when you ONLY want to display a single
// document. Once upon a golden time, you could figure this out in the constructor (you had
// a qApp argument). Alas, no longer. But if setDocument(...) gets called (i.e NOT list mode)
// a file gets passed to service_lnk. By the time the resizeEvent(...) gets called, we can figure
// out if this has happened.
// If we DON'T do this, we drag in a whole lot of extra documents, and end up displaying the first
// in that list prior to displaying the one we really want (along with other undesirable bugs)...
void PhotoEditUI::init(bool listMode)
{
    // Construct widget stack (only in list mode for phone)
    if ( listMode ) {
        // We're constructing for a list of documents, rather than just trying
        // to display a single picture.
        // For a list of documents, it makes sense to create the slide show. For a single
        // document, it does not.
        // FIXME: Don't create the slideshow until we need it
        slide_show = new SlideShow( this );
        slide_show_dialog = new SlideShowDialog( this );
        slide_show_ui = new SlideShowUI(0);
        slide_show_ui->setWindowTitle( windowTitle() );

        // Update image when slide show has changed
        connect( slide_show, SIGNAL(changed(QContent)),
                 slide_show_ui, SLOT(setImage(QContent)) );
        // Stop slide show when slide show ui pressed
        connect( slide_show_ui, SIGNAL(pressed()), slide_show, SLOT(stop()) );
        // Show selector when slide show has stopped
        connect( slide_show, SIGNAL(stopped()), this, SLOT(exitCurrentUIState()) );

        // Construct image selector
        widget_stack->addWidget( image_selector = new QImageDocumentSelector( this ) );

        if ( !service_category.acceptAll() ) {
            image_selector->setFilter(
                QContentFilter( QContent::Document ) &
                QContentFilter( QContentFilter::MimeType, "image/*" ) &
                QContentFilter( service_category ) );
        }

        connect( image_selector, SIGNAL(documentSelected(QContent)),
                 this, SLOT(setViewSingle()) );
        connect( image_selector, SIGNAL(documentsChanged()),
            this, SLOT(toggleActions()) );

        // Construct context menu for selector ui
        selector_menu = QSoftMenuBar::menuFor( image_selector );
        separator_action = selector_menu->insertSeparator( selector_menu->actions().first() );
        // Add properties item to selector menu
        properties_action = new QAction( QIcon( ":icon/info" ), tr( "Properties" ), this );
        properties_action->setVisible( false );
        connect( properties_action, SIGNAL(triggered()), this, SLOT(launchPropertiesDialog()) );
        selector_menu->insertAction( separator_action, properties_action );
        // Add print item to selector menu
        print_action = new QAction( QIcon( ":icon/beam" ), tr( "Print" ), this );
        print_action->setVisible( false );
        connect( print_action, SIGNAL(triggered()), this, SLOT(printImage()) );
        selector_menu->insertAction( properties_action, print_action );
        // Add beam item to selector menu
        beam_action = new QAction( QIcon( ":icon/beam" ), tr( "Beam" ), this );
        beam_action->setVisible( false );
        connect( beam_action, SIGNAL(triggered()), this, SLOT(beamImage()) );
        selector_menu->insertAction( print_action, beam_action );
        // Add delete item to selector menu
        delete_action = new QAction( QIcon( ":icon/trash" ), tr( "Delete" ), this );
        delete_action->setVisible( false );
        connect( delete_action, SIGNAL(triggered()), this, SLOT(deleteImage()) );
        selector_menu->insertAction( beam_action, delete_action );
        // Add edit item to selector menu
        edit_action = new QAction( QIcon( ":icon/edit" ), tr( "Edit" ), this );
        edit_action->setVisible( false );
        connect( edit_action, SIGNAL(triggered()), this, SLOT(editCurrentSelection()) );
        selector_menu->insertAction( delete_action, edit_action );
        // Add slide show item to selector menu
        slide_show_action = new QAction( QIcon( ":icon/slideshow" ), tr( "Slide Show..." ), this );
        slide_show_action->setVisible( true );
        connect( slide_show_action, SIGNAL(triggered()), this, SLOT(launchSlideShowDialog()) );
        selector_menu->insertAction( edit_action, slide_show_action );
    }

    // Construct image io
    image_io = new ImageIO( this );

    // Construct image processor
    image_processor = new ImageProcessor( image_io, this );

    // Construct image_ui or editor ui
    image_ui = new ImageUI(image_processor);
    widget_stack->addWidget(image_ui);

    // Construct image ui controls
    QVBoxLayout *box = new QVBoxLayout;
    box->setMargin(0);
    // Construct region selector
    region_selector = new RegionSelector( image_ui );
    box->addWidget( region_selector );

    if( Qtopia::mousePreferred() ) {
        connect( region_selector, SIGNAL(pressed()),
            this, SLOT(exitCurrentEditorState()) );
        connect( region_selector, SIGNAL(canceled()),
            this, SLOT(exitCurrentEditorState()) );
    }

    connect( region_selector, SIGNAL(selected()),
        this, SLOT(cropImage()) );
    connect( region_selector, SIGNAL(selected()),
        this, SLOT(exitCurrentEditorState()) );

    QGridLayout *grid_layout = new QGridLayout;
    grid_layout->setColumnStretch( 1, 1 );
    grid_layout->setRowStretch( 1, 1 );
    // Construct brightness control
    brightness_slider = new Slider( -70, 70, 0, 0, region_selector );
    grid_layout->addWidget( brightness_slider, 2, 1 );
    brightness_slider->hide();
    connect( brightness_slider, SIGNAL(selected()),
        this, SLOT(exitCurrentEditorState()) );
    connect( brightness_slider, SIGNAL(valueChanged(int)),
        this, SLOT(setBrightness(int)) );

    // Construct zoom control
    zoom_slider = new Slider( 100, 200, 10, 0, region_selector );
    zoom_slider->setSingleStep( 5 );
    grid_layout->addWidget( zoom_slider, 2, 1 );
    zoom_slider->hide();
    connect( zoom_slider, SIGNAL(selected()),
        this, SLOT(exitCurrentEditorState()) );
    connect( zoom_slider, SIGNAL(valueChanged(int)),
        this, SLOT(setZoom(int)) );

    // Construct navigator
    navigator = new Navigator( image_ui );
    grid_layout->addWidget( navigator, 2, 2 );

    region_selector->setLayout( grid_layout );
    image_ui->setLayout( box );

    // Clear context bar
    QSoftMenuBar::setLabel( image_ui, Qt::Key_Select, QSoftMenuBar::NoLabel );

    // Construct context menu for image ui
    QMenu *context_menu = QSoftMenuBar::menuFor( image_ui );
    QSoftMenuBar::setHelpEnabled( image_ui, true );

    context_menu->addAction( QIcon( ":icon/cut" ), tr( "Crop" ), this, SLOT(enterCrop()) );
    context_menu->addAction( QIcon( ":icon/color" ), tr( "Brightness" ), this, SLOT(enterBrightness()) );
    QAction *rotate_action = context_menu->addAction( QIcon( ":icon/rotate" ), tr( "Rotate" ), image_processor, SLOT(rotate()) );
    connect( rotate_action, SIGNAL(triggered()), this, SLOT(exitCurrentEditorState()) );
    context_menu->addSeparator();
    context_menu->addAction( QIcon( ":icon/find" ), tr( "Zoom" ), this, SLOT(enterZoom()) );
    // TODO -- Full screen mode has been removed from 4.2 until further notice, since it offers
    // nothing at this point in time.
    context_menu->addAction( QIcon( ":icon/fullscreen" ), tr( "Full Screen" ), this, SLOT(enterFullScreen()) );
    context_menu->addSeparator();
    context_menu->addAction( QIcon( ":icon/cancel" ), tr( "Cancel" ), this, SLOT(cancelEdit()) );

    connect( &selector_image, SIGNAL(rightsExpired(QDrmContent)),
              this,           SLOT  (setViewThumbnail()) );
    connect( &selector_image, SIGNAL(rightsExpired(QDrmContent)),
              this,           SLOT  (enterSelector()) );

    toggleActions();
    widget_stack->setCurrentIndex( widget_stack->indexOf( image_selector ) );
}

void PhotoEditUI::showEvent(QShowEvent *event )
{
    if ( !image_io ) {
        init(service_lnk.fileName().isNull());
    }
    QWidget::showEvent( event );
}

void PhotoEditUI::setDocument( const QString& lnk )
{
    service_lnk = QContent(lnk);

    QTimer::singleShot( 0, this, SLOT(processSetDocument()) );
}

void PhotoEditUI::editImage( const QDSActionRequest& request )
{
    showMaximized();
    currEditImageRequest = new QDSActionRequest( request );
    QDataStream stream( currEditImageRequest->requestData().toIODevice() );
    QPixmap orig;
    stream >> orig;
    service_image = orig.toImage();

    QTimer::singleShot( 0, this, SLOT(processGetImage()) );
}

void PhotoEditUI::appMessage( const QString& msg, const QByteArray& data )
{
    if( msg == "getImage(QString,QString,int,int,QString)" ) {
        QDataStream stream( data );
        QString filename;
        stream >> service_channel >> service_id >> service_width >> service_height >> filename;
        if ( filename.isEmpty() ) {
            service_image = QImage();
        } else {
            service_image = QImage( filename );
            QFile::remove( filename );
        }

        QTimer::singleShot( 0, this, SLOT(processGetImage()) );
    }
}

void PhotoEditUI::processSetDocument()
{
    only_editor = true;
    clearEditor();
    qApp->processEvents();
    current_image = service_lnk;
    enterEditor();
}

void PhotoEditUI::processGetImage()
{
    only_editor = true;
    clearEditor();
    qApp->processEvents();
    service_requested = true;
    if( service_image.isNull() ) enterSelector();
    else enterEditor();
}

// toggle the various actions to represent current state of the control
void PhotoEditUI::toggleActions()
{
    if ( !image_selector )
        return;

    // If there are images in the visible collection, enable actions
    // Otherwise, disable action
    bool b = image_selector->documents().count();

    switch( image_selector->viewMode() ) {
        case QImageDocumentSelector::Single:
            edit_action->setVisible( true );
            beam_action->setVisible( true );
            print_action->setVisible( true );
            delete_action->setVisible( true );
            properties_action->setVisible( true );
            slide_show_action->setVisible( false );
            QSoftMenuBar::setLabel( image_selector, Qt::Key_Select, QSoftMenuBar::NoLabel );
            break;
        case QImageDocumentSelector::Thumbnail:
            slide_show_action->setVisible( b );
            {
                // If an image is selected, then enable single image functions
                bool valid = b && image_selector->currentDocument().isValid();
                edit_action->setVisible( valid );
                beam_action->setVisible( valid );
                print_action->setVisible( valid );
                delete_action->setVisible( valid );
                properties_action->setVisible( valid );
            }
            QSoftMenuBar::setLabel( image_selector, Qt::Key_Select,
                                    b ? QSoftMenuBar::View : QSoftMenuBar::NoLabel );
            break;
    }

    separator_action->setVisible( b );
}

void PhotoEditUI::enterSelector()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::enterSelector() being called when there is NO image selector.");
        return;
    }

    // Clear current image
    current_image = QContent();
    // Raise selector to top of stack
    widget_stack->setCurrentIndex( widget_stack->indexOf( image_selector ) );
    toggleActions();
    image_selector->setFocus();
    ui_state = SELECTOR;
}

void PhotoEditUI::enterSlideShow()
{
    if ( !slide_show || !slide_show_ui ) {
        qWarning("PhotoEditUI::toggleActions() being called when there is NO slide show.");
        return;
    }
    if ( !image_selector ) {
        qWarning("PhotoEditUI::toggleActions() being called when there is NO image selector.");
        return;
    }

    // Set slide show collection from currently visible collection in selector
    slide_show->setCollection( image_selector->documents() );
    // Set first image in slideshow to currently selected image in selector
    slide_show->setFirstImage( image_selector->currentDocument() );
    // Show slide show ui in full screen
    slide_show_ui->showMaximized();
    // Start slideshow
    slide_show->start();
    ui_state = SLIDE_SHOW;
    QtopiaApplication::setPowerConstraint( QtopiaApplication::Disable );
}

void PhotoEditUI::enterEditor()
{
#define LIMIT( X, Y, Z ) ( (X) > (Y) ? (X) > (Z) ? (Z) : (X) : (Y) )
#define REDUCTION_RATIO( dw, dh, sw, sh ) \
    ( (dw)*(sh) > (dh)*(sw) ? (double)(dh)/(double)(sh) : \
    (double)(dw)/(double)(sw) )

    if ( navigator ) {
        navigator->hide();
    }
    if ( image_ui ) {
        image_ui->setEnabled( false );
    }

    // Raise editor to top of widget stack
    if ( widget_stack ) {
        widget_stack->setCurrentIndex( widget_stack->indexOf( image_ui ) );
    }
    ui_state = EDITOR;

    qApp->processEvents();

    // Update image io with current image
    ImageIO::Status status;
    if( service_requested && !service_image.isNull() && image_io ) {
        status = image_io->load( service_image );
    } else if ( !current_image.isValid() ) {
        // I believe this is because it is possible to get in here before
        // an image has been established.
        status = ImageIO::LOAD_ERROR;
    } else if( selector_image.requestLicense( current_image ) ) {
        status = image_io->load(current_image);
    } else {
        status = ImageIO::LOAD_ERROR;
    }

    switch( status ) {
        case ImageIO::REDUCED_SIZE:
            QMessageBox::information( 0, tr( "Scaled Image" ), tr( "<qt>Unable to load complete image.  A scaled copy has been opened instead.</qt>" ) );
        case ImageIO::NORMAL:
        {
            // Initialize editor controls
            brightness_slider->setValue( 0 );
            // Zoom to fit image in screen
            QSize size = image_io->size();
            QSize view = image_ui->size();
            disconnect( zoom_slider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)) );
            if( size.width() > view.width() || size.height() > view.height() ) {
                double ratio = REDUCTION_RATIO( view.width(), view.height(), size.width(), size.height() );
                ratio = LIMIT( ratio, 0.1, 1.0 );
                image_processor->setZoom( ratio );
                zoom_slider->setMaximum( 200 );
                zoom_slider->setValue( (int) ( log( ratio * 100 ) / log( 10.0 ) * 100 ) );
            } else {
                double ratio = REDUCTION_RATIO( view.width(), view.height(), size.width(), size.height() );
                ratio = LIMIT( ratio, 1.0, 10.0 );
                image_processor->setZoom( 1.0 );
                zoom_slider->setMaximum( (int) ( log( ratio * 100 ) / log( 10.0 ) * 100 ) );
                zoom_slider->setValue( 200 );
            }
            connect( zoom_slider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)) );
            image_ui->reset();
            image_ui->setEnabled( true );
            navigator->show();
            navigator->setFocus();
            selector_image.renderStarted();
        }
        break;

        case ImageIO::SIZE_ERROR:
        case ImageIO::LOAD_ERROR:
            QMessageBox::warning( 0, tr( "Load Error" ), tr( "<qt>Unable to load image.</qt>" ) );
            if ( only_editor )
                close();
            else
                enterSelector();
        break;

        case ImageIO::DEPTH_ERROR:
            QMessageBox::warning( 0, tr( "Depth Error" ), tr( "<qt>Image depth is not supported.</qt>" ) );
            if ( only_editor )
                close();
            else
                enterSelector();
        break;
    }
}

void PhotoEditUI::enterZoom()
{
    // Hide navigator
    navigator->hide();
    // Show zoom control
    zoom_slider->show();
    zoom_slider->setFocus();
    editor_state = ZOOM;
}

void PhotoEditUI::enterBrightness()
{
    // Hide navigator
    navigator->hide();
    // Show brightness control
    brightness_slider->show();
    brightness_slider->setFocus();
    editor_state = BRIGHTNESS;
}

void PhotoEditUI::enterCrop()
{
    // Hide navigator
    navigator->hide();
    // Enable selection in region selector
    region_selector->reset();
    region_selector->setEnabled( true );
    region_selector->setFocus();
    editor_state = CROP;
}

void PhotoEditUI::enterFullScreen()
{
    navigator->hide();
    // Show editor view in full screen
    QString title = windowTitle();

    setWindowTitle( QLatin1String( "_allow_on_top_" ) );
    showFullScreen();
    raise();

    setWindowTitle( title );

    editor_state = FULL_SCREEN;
}

void PhotoEditUI::setViewThumbnail()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::setViewThumbnail() being called when there is NO image selector.");
        return;
    }

    // If image selector not in multi, change to single and update context menu
    if( image_selector->viewMode() != QImageDocumentSelector::Thumbnail )
    {
        image_selector->setViewMode( QImageDocumentSelector::Thumbnail );
    }
    toggleActions();
}

void PhotoEditUI::setViewSingle()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::setViewSingle() being called when there is NO image selector.");
        return;
    }

    // If image selector not in single, change to single and update context menu
    if( image_selector->viewMode() != QImageDocumentSelector::Single )
    {
        image_selector->setViewMode( QImageDocumentSelector::Single );
    }
    toggleActions();
}

void PhotoEditUI::launchSlideShowDialog()
{
    if ( !slide_show_dialog ) {
        qWarning("PhotoEditUI::launchSlideShowDialog() being called when there is NO slide show.");
        return;
    }

    // If slide show dialog accepted, start slideshow
    if( QtopiaApplication::execDialog( slide_show_dialog, true ) ) {
        // Set slide show options
        slide_show_ui->setDisplayName( slide_show_dialog->isDisplayName() );
        slide_show->setSlideLength( slide_show_dialog->slideLength() );
        slide_show->setLoopThrough( slide_show_dialog->isLoopThrough() );
        enterSlideShow();
    }
}

void PhotoEditUI::launchPropertiesDialog()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::launchPropertiesDialog() being called when there is NO image selector.");
        return;
    }

    QContent doc( image_selector->currentDocument() );
    QDocumentPropertiesDialog dialog( doc );
    // Launch properties dialog with current image
    QtopiaApplication::execDialog( &dialog );
}

bool PhotoEditUI::exitCurrentUIState()
{
    switch( ui_state ) {
    case SELECTOR:
        if ( !image_selector ) {
            qWarning("PhotoEditUI::exitCurrentUIState() being called for context requiring image selector.");
            break;
        }

        // If in single view and there are images, change to thumbnail and update context menu
        // Otherwise, close application
        if ( image_selector->viewMode() == QImageDocumentSelector::Single &&
             image_selector->documents().count() )
            setViewThumbnail();
        else
            return true;
        break;
    case SLIDE_SHOW:
        if ( !slide_show_ui ) {
            qWarning("PhotoEditUI::exitCurrentUIState() being called for context requiring slide show.");
            break;
        }
        QtopiaApplication::setPowerConstraint( QtopiaApplication::Enable );
        // Return from slide show
        slide_show_ui->hide();
        enterSelector();
        break;
    case EDITOR:
        selector_image.releaseLicense();

        if( !edit_canceled ) {
            if( service_requested ) {
                sendValueSupplied();
            } else {
                saveChanges();
            }
        }
        if( only_editor ) {
            if( service_requested && service_image.isNull() && edit_canceled ) {
                enterSelector();
            } else {
                return true;
            }
        } else {
            enterSelector();
        }
        edit_canceled = false;
        break;
    }

    return false;
}

void PhotoEditUI::exitCurrentEditorState()
{
    switch( editor_state ) {
    // If in view, no change
    case VIEW:
        editor_state_changed = false;
        break;
    // If in full screen, return from full screen
    case FULL_SCREEN:
        // Set editor central widget to editor view
        navigator->show();
        navigator->setFocus();
        showMaximized();
        editor_state = VIEW;
        editor_state_changed = true;
        break;
    // If in zoom, hide zoom control
    case ZOOM:
        zoom_slider->hide();
        // Show navigator
        navigator->show();
        navigator->setFocus();
        editor_state = VIEW;
        editor_state_changed = true;
        break;
    // If in brightness, hide brightness control
    case BRIGHTNESS:
        brightness_slider->hide();
        // Show navigator
        navigator->show();
        navigator->setFocus();
        editor_state = VIEW;
        editor_state_changed = true;
        break;
    // If in crop, disable region selector and show navigator
    case CROP:
        region_selector->setEnabled( false );
        region_selector->update();
        // Show navigator
        navigator->show();
        navigator->setFocus();
        editor_state = VIEW;
        editor_state_changed = true;
        break;
    };
}

void PhotoEditUI::setZoom( int x )
{
    image_processor->setZoom( pow( 10.0, (double)x / 100.0 ) / 100.0 );
}

void PhotoEditUI::setBrightness( int x )
{
    image_processor->setBrightness( (double)x / 100.0 );
}

void PhotoEditUI::editCurrentSelection()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::editCurrentSelection() being called when there is NO image selector.");
        return;
    }

    // Retrieve current selection from image selector and open for editing
    current_image = image_selector->currentDocument();

    enterEditor();
}

void PhotoEditUI::cancelEdit()
{
    edit_canceled = true;

    if( exitCurrentUIState() )
        close();
}

void PhotoEditUI::cropImage()
{
    // Ensure cropping region is valid
    QRect region( region_selector->region() );
    if( region.isValid() ) {
        // Retrieve region from region selector
        // Calculate cropped viewport and crop
        image_processor->crop( image_ui->viewport(
            region_selector->region() ) );
        // Reset viewport
        image_ui->reset();
    }
}

void PhotoEditUI::beamImage()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::beamImage() being called when there is NO image selector.");
        return;
    }

    // Send current image over IR link
    QContent image = image_selector->currentDocument();
    QtopiaSendVia::sendFile(this, image);
}

void PhotoEditUI::printImage()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::printImage() being called when there is NO image selector.");
        return;
    }

    QContent image = image_selector->currentDocument();
    QtopiaServiceRequest srv( "Print", "print(QString,QString)" );
    srv << image.fileName();
    srv << (image.mimeTypes().count() ? image.mimeTypes().at(0) : QString());
    srv.send();
}

void PhotoEditUI::deleteImage()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::deleteImage() being called when there is NO image selector.");
        return;
    }

    // Retrieve currently highlighted image from selector
    QContent image = image_selector->currentDocument();

    // Launch confirmation dialog
    // If deletion confirmed, delete image
    if( QMessageBox::information( this, tr( "Delete" ), tr( "<qt>Are you sure "
        "you want to delete %1?</qt>"," %1 = file name" ).arg(image.name()),
        QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
        image.removeFiles();
}

void PhotoEditUI::contentChanged( const QContentIdList &idList, const QContent::ChangeType type )
{
    if ( !idList.contains(current_image.id()) )
        return;

    // If content is current image and has been deleted, show selector
    if ( QContent::Removed == type ) {
        if ( only_editor )
            close();
        else
            enterSelector();
    }
}

void PhotoEditUI::keyPressEvent( QKeyEvent *e )
{
    if( e->key() == Qt::Key_Back )
    {
        if( ui_state == EDITOR && editor_state != VIEW )
        {
            exitCurrentEditorState();
        }
        else if( exitCurrentUIState() )
        {
            close();
        }
        e->accept();
    }
    else
        QWidget::keyPressEvent( e );
}

void PhotoEditUI::clearEditor()
{
    if ( navigator )
        navigator->hide();
    if ( image_ui ) {
        image_ui->setEnabled( false );
        image_ui->repaint();
    }
}

void PhotoEditUI::saveChanges()
{
    // If image was changed, prompt user to save changes
    if( image_processor->isChanged() && current_image.drmState() == QContent::Unprotected ) {
        // If save supported, prompt user to overwrite original
        // Otherwise, save as new file
        bool overwrite = false;
        if( image_io->isSaveSupported() ) {
            if ( image_io->isReadOnly() ) {
                QMessageBox::warning( this,
                    tr( "Read-Only File" ),
                    tr( "<qt>Saving a copy of the read-only file.</qt>" ) );
            } else {
                if( QMessageBox::information( this, tr( "Save Changes " ),
                    tr( "<qt>Do you want to overwrite the original?</qt>" ),
                    QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
                    overwrite = true;
            }
        } else {
            QByteArray format = image_io->format();
            QMessageBox::warning( this,
                tr( "Saving %1" ).arg( format.constData() ),
                tr( "<qt>Saving as %1 is not supported. "
                    "Using the default format instead.</qt>" )
                    .arg( format.constData() ) );
        }

        QImage image = image_processor->image();
        // Attempt to save changes
        if ( !image_io->save( image, overwrite ) ) {
             QMessageBox::warning(
                this,
                tr( "Save failed" ),
                tr( "<qt>Your edits were not saved.</qt>" ) );
        }
    }
}

void PhotoEditUI::sendValueSupplied()
{
    if ( currEditImageRequest != 0 ) {
        // Create edited picture data object
        QByteArray editedArray;
        {
            QDataStream stream( &editedArray, QIODevice::WriteOnly );
            stream << QPixmap::fromImage( image_processor->image() );
        }
        QDSData edited( editedArray, QMimeType( "image/x-qpixmap" ) );

        // Respond to request, and cleanup the request
        currEditImageRequest->respond( edited );
        delete currEditImageRequest;
        currEditImageRequest = 0;
    } else {
        QtopiaIpcEnvelope e( service_channel, "valueSupplied(QString,QString)" );
        QString path = Qtopia::applicationFileName("Temp", service_id);
        QImage img = image_processor->image( QSize( service_width, service_height ) );
        img.save(path,"JPEG");
        e << service_id << path;
    }
}

void PhotoEditUI::rightsExpired( const QDrmContent& )
{
    if( only_editor )
        close();
    else
        enterSelector();
}

/*!
    \service PhotoEditService PhotoEdit
    \brief Provides the Qtopia PhotoEdit service.

    The \i PhotoEdit service enables applications to access features
    within the photo editing application.
*/

/*!
    \internal
*/
PhotoEditService::~PhotoEditService()
{
}

/*!
    Display the photo editor's main document list and show the documents
    within \a category.

    This slot corresponds to the QCop service message
    \c{PhotoEdit::showCategory(QString)}.
*/
void PhotoEditService::showCategory( const QString& category )
{
    mParent->service_category = QCategoryFilter(category);

    if ( mParent->image_selector ) {
        mParent->image_selector->setFilter(
                QContentFilter( QContent::Document ) &
                QContentFilter( QContentFilter::MimeType, "image/*" ) &
                QContentFilter( mParent->service_category ) );

        mParent->enterSelector();

    }
    mParent->showMaximized();
}

/*!
    Allows users to edit the image contained within \a request using the photo
    editor dialog.

    This slot corresponds to a QDS service with a request and response data type
    of "image/x-qpixmap".

    This slot corresponds to the QCop service message
    \c{PhotoEdit::editImage(QDSActionRequest)}.
*/
void PhotoEditService::editImage( const QDSActionRequest& request )
{
    mParent->editImage( request );
}
