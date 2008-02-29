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

#include "photoeditui.h"


#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qdocumentproperties.h>
#include <qtopiaipcenvelope.h>
#include <qtopiasendvia.h>
#include <qtopiaservices.h>
#include <qcontent.h>

#include <QPoint>
#include <QMessageBox>
#include <QGridLayout>
#include <QSize>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QDebug>
#include <QDSActionRequest>
#include <QDSData>
#include <QPixmap>
#include <QMimeType>

#include <cmath>

class PhotoEditStackedWidget : public QStackedWidget
{
public:
    PhotoEditStackedWidget( QWidget *parent = 0 )
        : QStackedWidget( parent )
    {
    }
protected:
    void closeEvent( QCloseEvent *e )
    {
        parentWidget()->close();
        e->ignore();
    }

    bool event( QEvent *e )
    {
        return QStackedWidget::event( e );
    }
};

PhotoEditUI::PhotoEditUI( QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f )
    , ui_state( SELECTOR )
    , editor_state( VIEW )
    , only_editor( false )
    , service_requested( false )
    , selector_image( QDrmRights::Display )
#ifdef QTOPIA_PHONE
    , selector_menu(0)
    , separator_action(0)
    , properties_action(0)
    , beam_action(0)
    , print_action(0)
    , delete_action(0)
    , edit_action(0)
    , slide_show_action(0)
#else
    , selector_ui(0)
    , selector_menu(0)
#endif
    , image_selector(0)
    , editor_ui(0)
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
    , currEditImageRequest( 0 )
{
#ifdef QTOPIA_PHONE
    was_fullscreen = false;
    edit_canceled = false;
    close_ok = false;
    editor_state_changed = false;
#endif

    setWindowTitle( tr( "Pictures" ) );

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout( layout );

    // Respond to service requests
    connect( qApp, SIGNAL( appMessage( const QString&, const QByteArray& ) ),
        this, SLOT( appMessage( const QString&, const QByteArray& ) ) );

    new PhotoEditService( this );

    // Respond to file changes
    connect( qApp,
        SIGNAL(contentChanged(const QContentIdList&,const QContent::ChangeType)),
        this,
        SLOT( contentChanged(const QContentIdList&,const QContent::ChangeType)));

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
    bool phone = false;
#ifdef QTOPIA_PHONE
    phone = true;
#endif

    // Construct widget stack
    if ( listMode || !phone ) {
        // We're constructing for a list of documents, rather than just trying
        // to display a single picture.
        widget_stack = new PhotoEditStackedWidget;
        layout()->addWidget( widget_stack );

        // For a list of documents, it makes sense to create the slide show. For a single
        // document, it does not.
        slide_show = new SlideShow( this );
        slide_show_dialog = new SlideShowDialog( this );
        slide_show_ui = new SlideShowUI(0);
        // Update image when slide show has changed
        connect( slide_show, SIGNAL( changed( const QContent& ) ),
                 slide_show_ui, SLOT( setImage( const QContent& ) ) );
        // Stop slide show when slide show ui pressed
        connect( slide_show_ui, SIGNAL( pressed() ), slide_show, SLOT( stop() ) );
        // Show selector when slide show has stopped
        connect( slide_show, SIGNAL( stopped() ),
            this, SLOT( exitCurrentUIState() ) );
    }

#ifndef QTOPIA_PHONE
    // Construct selector ui
    widget_stack->addWidget( selector_ui = new SelectorUI( widget_stack ) );
    // When slide show selected launch slide show dialog
    connect( selector_ui, SIGNAL( slideShow() ),
        this, SLOT( launchSlideShowDialog() ) );
    connect( selector_ui, SIGNAL( edit() ),
        this, SLOT( editCurrentSelection() ) );
    connect( selector_ui, SIGNAL( beam() ),
        this, SLOT( beamImage() ) );
    connect( selector_ui, SIGNAL( remove() ),
        this, SLOT( deleteImage() ) );
    // When properties selected launch properties dialog
    connect( selector_ui, SIGNAL( properties() ),
        this, SLOT( launchPropertiesDialog() ) );
#endif

    // Construct image selector
#ifdef QTOPIA_PHONE
    if ( listMode ) {
        widget_stack->addWidget( image_selector = new QImageDocumentSelector( widget_stack ) );
    }
#else
    image_selector = new QImageDocumentSelector( selector_ui );
    selector_ui->setCentralWidget( image_selector );
    connect( image_selector, SIGNAL( documentHeld( const QContent&, const QPoint& ) ),
        this, SLOT( launchPopupMenu( const DocLnk&, const QPoint& ) ) );
#endif

    if ( image_selector ) {
        connect( image_selector, SIGNAL(documentSelected( const QContent& )),
                 this, SLOT(setViewSingle()) );
        connect( image_selector, SIGNAL( documentsChanged() ),
            this, SLOT( toggleActions() ) );
    }

#ifdef QTOPIA_PHONE
    if ( image_selector ) {
        // Construct context menu for selector ui
        selector_menu = QSoftMenuBar::menuFor( image_selector );
        separator_action = selector_menu->insertSeparator( selector_menu->actions().first() );
        QMenu *menu = (QMenu*)selector_menu;
        // Add properties item to selector menu
        properties_action = new QAction( QIcon( ":icon/info" ), tr( "Properties" ), this );
        properties_action->setVisible( false );
        connect( properties_action, SIGNAL( triggered() ), this, SLOT( launchPropertiesDialog() ) );
        menu->insertAction( separator_action, properties_action );
        // Add print item to selector menu
        print_action = new QAction( QIcon( ":icon/beam" ), tr( "Print" ), this );
        print_action->setVisible( false );
        connect( print_action, SIGNAL( triggered() ), this, SLOT( printImage() ) );
        menu->insertAction( properties_action, print_action );
        // Add beam item to selector menu
        beam_action = new QAction( QIcon( ":icon/beam" ), tr( "Beam" ), this );
        beam_action->setVisible( false );
        connect( beam_action, SIGNAL( triggered() ), this, SLOT( beamImage() ) );
        menu->insertAction( print_action, beam_action );
        // Add delete item to selector menu
        delete_action = new QAction( QIcon( ":icon/trash" ), tr( "Delete" ), this );
        delete_action->setVisible( false );
        connect( delete_action, SIGNAL( triggered() ), this, SLOT( deleteImage() ) );
        menu->insertAction( beam_action, delete_action );
        // Add edit item to selector menu
        edit_action = new QAction( QIcon( ":icon/edit" ), tr( "Edit" ), this );
        edit_action->setVisible( false );
        connect( edit_action, SIGNAL( triggered() ), this, SLOT( editCurrentSelection() ) );
        menu->insertAction( delete_action, edit_action );
        // Add slide show item to selector menu
        slide_show_action = new QAction( QIcon( ":icon/slideshow" ), tr( "Slide Show..." ), this );
        slide_show_action->setVisible( true );
        connect( slide_show_action, SIGNAL( triggered() ), this, SLOT( launchSlideShowDialog() ) );
        menu->insertAction( edit_action, slide_show_action );
    }
#else

    selector_menu = new QMenu( this );
    selector_menu->addAction( tr( "Edit" ), this, SLOT( editCurrentSelection() ) );
    selector_menu->addAction( tr( "Delete" ), this, SLOT( deleteImage() ) );
    selector_menu->addAction( tr( "Beam" ), this, SLOT( beamImage() ) );
    selector_menu->addAction( tr( "Print" ), this, SLOT( printImage() ) );
    selector_menu->addAction( tr( "Properties" ), this, SLOT( launchPropertiesDialog() ) );

#endif

    // Construct image io
    image_io = new ImageIO( this );

    // Construct image processor
    image_processor = new ImageProcessor( image_io, this );

    // Construct image_ui or editor ui
#ifdef QTOPIA_PHONE
    if ( widget_stack ) {
        widget_stack->addWidget( image_ui = new ImageUI( image_processor ) );
    } else {
        image_ui = new ImageUI(image_processor);
        layout()->addWidget(image_ui);
    }
#else
    widget_stack->addWidget( editor_ui = new EditorUI( widget_stack ) );
    image_ui = new ImageUI( image_processor, editor_ui ); // editor_ui is the parent of image_ui
#endif

    image_ui->installEventFilter( this );

    // Construct image ui controls
    QVBoxLayout *box = new QVBoxLayout;
    box->setMargin(0);
    // Construct region selector
    region_selector = new RegionSelector( image_ui );
    box->addWidget( region_selector );

#ifdef QTOPIA_PHONE
    if( Qtopia::mousePreferred() ) {
#endif
        connect( region_selector, SIGNAL( pressed() ),
            this, SLOT( exitCurrentEditorState() ) );
        connect( region_selector, SIGNAL( canceled() ),
            this, SLOT( exitCurrentEditorState() ) );
#ifdef QTOPIA_PHONE
    }
#endif
    connect( region_selector, SIGNAL( selected() ),
        this, SLOT( cropImage() ) );
    connect( region_selector, SIGNAL( selected() ),
        this, SLOT( exitCurrentEditorState() ) );

    QGridLayout *grid_layout = new QGridLayout;
    grid_layout->setColumnStretch( 1, 1 );
    grid_layout->setRowStretch( 1, 1 );
    // Construct brightness control
    brightness_slider = new Slider( -70, 70, 0, 0, region_selector );
    grid_layout->addWidget( brightness_slider, 2, 1 );
    brightness_slider->hide();
#ifdef QTOPIA_PHONE
    connect( brightness_slider, SIGNAL( selected() ),
        this, SLOT( exitCurrentEditorState() ) );
#endif
    connect( brightness_slider, SIGNAL( valueChanged( int ) ),
        this, SLOT( setBrightness( int ) ) );

    // Construct zoom control
    zoom_slider = new Slider( 100, 200, 10, 0, region_selector );
    zoom_slider->setSingleStep( 5 );
    grid_layout->addWidget( zoom_slider, 2, 1 );
    zoom_slider->hide();
#ifdef QTOPIA_PHONE
    connect( zoom_slider, SIGNAL( selected() ),
        this, SLOT( exitCurrentEditorState() ) );
#endif
    connect( zoom_slider, SIGNAL( valueChanged( int ) ),
        this, SLOT( setZoom( int ) ) );

    // Construct navigator
    navigator = new Navigator( image_ui );
    grid_layout->addWidget( navigator, 2, 2 );

    region_selector->setLayout( grid_layout );
    image_ui->setLayout( box );

#ifdef QTOPIA_PHONE
    // Construct context menu for image ui
    QAction *crop_action = new QAction( QIcon( ":icon/cut" ), tr( "Crop" ), this );
    connect( crop_action, SIGNAL(triggered()),
        this, SLOT(enterCrop()) );

    QAction *brightness_action = new QAction( QIcon( ":icon/color" ), tr( "Brightness" ), this );
    connect( brightness_action, SIGNAL(triggered()),
        this, SLOT(enterBrightness()) );

    QAction *rotate_action = new QAction( QIcon( ":icon/rotate" ), tr( "Rotate" ), this );
    connect( rotate_action, SIGNAL(triggered()),
        image_processor, SLOT(rotate()) );
    connect( rotate_action, SIGNAL(triggered()),
        this, SLOT(exitCurrentEditorState()) );

    QAction *zoom_action = new QAction( QIcon( ":icon/find" ), tr( "Zoom" ), this );
    connect( zoom_action, SIGNAL(triggered()),
        this, SLOT(enterZoom()) );

    // TODO -- Full screen mode has been removed from 4.2 until further notice, since all it offers
    // nothing at this point in time.
    //QAction *fullscreen_action = new QAction( QIcon( ":icon/fullscreen" ), tr( "Full Screen" ), this );
    //connect( fullscreen_action, SIGNAL(triggered()),
    //    this, SLOT(enterFullScreen()) );

    // Clear context bar
    QSoftMenuBar::setLabel( image_ui, Qt::Key_Select, QSoftMenuBar::NoLabel );

    // Construct context menu
    QMenu *context_menu = QSoftMenuBar::menuFor( image_ui );
    QSoftMenuBar::setHelpEnabled( image_ui, true );
    context_menu->addAction( crop_action );
    context_menu->addAction( brightness_action );
    context_menu->addAction( rotate_action );
    context_menu->addSeparator();
    context_menu->addAction( zoom_action );
    //context_menu->addAction( fullscreen_action );  SEE CREATION OF fullscreen_action
    context_menu->addSeparator();

    // Ignore changes if edit is canceled
    context_menu->addAction( QIcon( ":icon/close" ),
        tr( "Cancel" ), this, SLOT(cancelEdit()) );
#else
    // Connect editor ui menu items
    connect( editor_ui, SIGNAL( open() ),
        this, SLOT( exitCurrentUIState() ) );
    connect( editor_ui, SIGNAL( open() ),
        this, SLOT( exitCurrentEditorState() ) );
    connect( editor_ui, SIGNAL( rotate() ),
        this, SLOT( exitCurrentEditorState() ) );

    // Connect imaging functions to image processor
     connect( editor_ui, SIGNAL( zoom() ),
        this, SLOT( enterZoom() ) );
    connect( editor_ui, SIGNAL( brightness() ),
        this, SLOT( enterBrightness() ) );
    connect( editor_ui, SIGNAL( crop() ),
        this, SLOT( enterCrop() ) );
    connect( editor_ui, SIGNAL( rotate() ),
        image_processor, SLOT( rotate() ) );
    connect( editor_ui, SIGNAL( fullScreen() ),
        this, SLOT( enterFullScreen() ) );
#endif

    connect( &selector_image, SIGNAL(rightsExpired(const QDrmContent&)),
              this,           SLOT  (setViewThumbnail()) );
    connect( &selector_image, SIGNAL(rightsExpired(const QDrmContent&)),
              this,           SLOT  (enterSelector()) );

    if ( widget_stack ) {
        toggleActions();
#ifdef QTOPIA_PHONE
        widget_stack->setCurrentIndex( widget_stack->indexOf( image_selector ) );
#else
        widget_stack->setCurrentIndex( widget_stack->indexOf( selector_ui ) );
#endif
    }
}

void PhotoEditUI::resizeEvent(QResizeEvent *event)
{
    if ( !image_io ) {
        // Finish creating the UI.
        init(service_lnk.file().isNull());
#ifdef QTOPIA_PHONE
        // ***HACK HACK*** This one is because (you guessed it) the size doesn't get set properly
        // for some reason, and you get the thing trying to display a huge image.
        if ( image_ui ) {
            image_ui->setFixedSize(size());
        }
        // ***HACK HACK HACKETY-HACK*** And the QStackedWidget doesn't seem to want to resize
        // to fit its containing layout. Or maybe, it's supposed to resize for it's children, & we
        // were actually meant to explicitly set the sizes of all those.
        if ( widget_stack ) {
            widget_stack->setFixedSize(size());
        }
#endif
    }

    QWidget::resizeEvent(event);
}

bool PhotoEditUI::eventFilter( QObject*, QEvent* e )
{
    if( e->type() == QEvent::WindowDeactivate ) {
        switch( ui_state ) {
        case EDITOR:
            if( editor_state == FULL_SCREEN ) {
                exitCurrentEditorState();
#ifdef QTOPIA_PHONE
                was_fullscreen = false;
#endif
            }
            break;
        default:
            // Ignore
            break;
        }
    }

    return false;
}

void PhotoEditUI::setDocument( const QString& lnk )
{
    service_lnk = QContent(lnk);

    // Ensure editor is raised
    // This wiil be done in enterEditor() and is causing layout
    // problems in at least phone edition so should probably be removed completely.
#ifndef QTOPIA_PHONE
    // If the following is called at this point, it will crash. If we trust the
    // preceding comment, we can remove it.
    /*
    if( ui_state != EDITOR ) {
        image_ui->setEnabled( false );
        if ( widget_stack ) {
            widget_stack->setCurrentIndex( widget_stack->indexOf( editor_ui ) );
        }
    }
    */
#endif
    QTimer::singleShot( 0, this, SLOT( processSetDocument() ) );
}

void PhotoEditUI::editImage( const QDSActionRequest& request )
{
    showMaximized();
#ifndef QTOPIA_PHONE
    // Respond to previous service request
    if( ui_state == EDITOR && service_requested ) sendValueSupplied();
    service_image = QImage();
#endif
    currEditImageRequest = new QDSActionRequest( request );
    QDataStream stream( currEditImageRequest->requestData().toIODevice() );
    QPixmap orig;
    stream >> orig;
    service_image = orig.toImage();

    QTimer::singleShot( 0, this, SLOT( processGetImage() ) );
}

void PhotoEditUI::appMessage( const QString& msg, const QByteArray& data )
{
    if( msg == "getImage(QString,QString,int,int,QString)" ) {
#ifndef QTOPIA_PHONE
        // Respond to previous service request
        if( ui_state == EDITOR && service_requested ) sendValueSupplied();
        service_image = QImage();
#endif
        QDataStream stream( data );
        QString filename;
        stream >> service_channel >> service_id >> service_width >>
            service_height >> filename;
        if ( filename.isEmpty() ) {
            service_image = QImage();
        } else {
            service_image = QImage( filename );
            QFile::remove( filename );
        }

        QTimer::singleShot( 0, this, SLOT( processGetImage() ) );
    }
}

void PhotoEditUI::processSetDocument()
{
#ifndef QTOPIA_PHONE
    interruptCurrentState();
    service_requested = false;
    if( !only_editor ) {
        editor_ui->removeFileItems();
#endif
        only_editor = true;
#ifndef QTOPIA_PHONE
    }
#endif
    clearEditor();
    qApp->processEvents();
    current_image = service_lnk;
    enterEditor();
}

void PhotoEditUI::processShowCategory()
{
#ifndef QTOPIA_PHONE
    interruptCurrentState();
    service_requested = false;
    if( only_editor ) {
        editor_ui->addFileItems();
#endif
        only_editor = false;
#ifndef QTOPIA_PHONE
    }
#endif

    if ( image_selector ) {
        image_selector->setFilter(
            QContentFilter( QContent::Document ) &
            QContentFilter( QContentFilter::MimeType, "image/*" ) &
            QContentFilter( service_category ) );

        // For some reason processing events here causes an assert in Qt
        //qApp->processEvents();
        enterSelector();
    } else {
        qWarning("PhotoEditUI::processShowCategory() being called when there is no image_selector.");
    }
}

void PhotoEditUI::processGetImage()
{
#ifndef QTOPIA_PHONE
    if( ui_state == EDITOR && !service_requested ) saveChanges();
    if( !only_editor && editor_ui ) {
        editor_ui->removeFileItems();
#endif
        only_editor = true;
#ifndef QTOPIA_PHONE
    }
#endif
    clearEditor();
    qApp->processEvents();
    service_requested = true;
    if( service_image.isNull() ) enterSelector();
    else enterEditor();
}

// toggle the various actions to represent current state of the control
void PhotoEditUI::toggleActions()
{
    if ( !image_selector ) {
        return;
    }

    // If there are images in the visible collection, enable actions
    // Otherwise, disable action
    bool b = image_selector->documents().count();
#ifdef QTOPIA_PHONE

    switch( image_selector->viewMode() ) {
        case QImageDocumentSelector::Single:
            edit_action->setVisible( true );
            beam_action->setVisible( true );
            print_action->setVisible( true );
            delete_action->setVisible( true );
            properties_action->setVisible( true );
            slide_show_action->setVisible( false );
            QSoftMenuBar::setLabel( image_selector,
                                    Qt::Key_Select,
                                    QSoftMenuBar::NoLabel );
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
            if( b ) QSoftMenuBar::setLabel( image_selector, Qt::Key_Select,
                QSoftMenuBar::View );
            else QSoftMenuBar::setLabel( image_selector, Qt::Key_Select,
                QSoftMenuBar::NoLabel );
            break;
    }

    separator_action->setVisible( b );

#else
    selector_ui->setEnabled( b );
#endif
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
#ifdef QTOPIA_PHONE
    widget_stack->setCurrentIndex( widget_stack->indexOf( image_selector ) );
    toggleActions();
    image_selector->setFocus();
#else
    widget_stack->setCurrentIndex( widget_stack->indexOf( selector_ui ) );
#endif
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
#ifdef QTOPIA_PHONE
    if ( widget_stack ) {
        widget_stack->setCurrentIndex( widget_stack->indexOf( image_ui ) );
    }
#else
    widget_stack->setCurrentIndex( widget_stack->indexOf( editor_ui ) );
    // Integration comment -- !! Do we REALLY need this?? It wasn't there before...
    editor_ui->show();
#endif
    ui_state = EDITOR;

    qApp->processEvents();

    // Update image io with current image
    ImageIO::Status status;
    if( service_requested && !service_image.isNull() && image_io ) {
        status = image_io->load( service_image );
    } else {
        if ( !(current_image.isValid()) ) {
            // I believe this is because it is possible to get in here before
            // an image has been established.
            status = ImageIO::NORMAL;
        } else if( selector_image.requestLicense( current_image ) ) {
            status = image_io->load(current_image);

        } else {
            status = ImageIO::LOAD_ERROR;
        }
    }

    switch( status ) {
    case ImageIO::NORMAL:
        {
            // Initialize editor controls
            brightness_slider->setValue( 0 );
            // Zoom to fit image in screen
            QSize size = image_io->size();
            QSize view = image_ui->size();
            disconnect( zoom_slider, SIGNAL( valueChanged( int ) ),
                this, SLOT( setZoom( int ) ) );
            if( size.width() > view.width() || size.height() > view.height() ) {
                double ratio = REDUCTION_RATIO( view.width(), view.height(),
                    size.width(), size.height() );
                ratio = LIMIT( ratio, 0.1, 1.0 );
                image_processor->setZoom( ratio );
                zoom_slider->setValue( (int) ( log( ratio * 100 ) /
                    log( 10.0 ) * 100 ) );
            } else {
                image_processor->setZoom( 1.0 );
                zoom_slider->setValue( 200 );
            }
            connect( zoom_slider, SIGNAL( valueChanged( int ) ),
                this, SLOT( setZoom( int ) ) );
            image_ui->reset();
            image_ui->setEnabled( true );
            navigator->show();
            navigator->setFocus();
            selector_image.renderStarted();
        }
        break;
    case ImageIO::LOAD_ERROR:
        QMessageBox::warning( 0, tr( "Load Error" ),
            tr( "<qt>Unable to load image.</qt>" ) );
        if( only_editor ) close();
        else enterSelector();
        break;
    case ImageIO::DEPTH_ERROR:
        QMessageBox::warning( 0, tr( "Depth Error" ),
            tr( "<qt>Image depth is not supported.</qt>" ) );
        if( only_editor ) close();
        else enterSelector();
        break;
    }
}

void PhotoEditUI::enterZoom()
{
#ifndef QTOPIA_PHONE
    exitCurrentEditorState();
#endif
    // Hide navigator
    navigator->hide();
    // Show zoom control
    zoom_slider->show();
#ifdef QTOPIA_PHONE
    zoom_slider->setFocus();
#endif
    editor_state = ZOOM;
}

void PhotoEditUI::enterBrightness()
{
#ifndef QTOPIA_PHONE
    exitCurrentEditorState();
#endif
    // Hide navigator
    navigator->hide();
    // Show brightness control
    brightness_slider->show();
#ifdef QTOPIA_PHONE
    brightness_slider->setFocus();
#endif
    editor_state = BRIGHTNESS;
}

void PhotoEditUI::enterCrop()
{
#ifndef QTOPIA_PHONE
    exitCurrentEditorState();
#endif
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
#ifndef QTOPIA_PHONE
    exitCurrentEditorState();
#endif
    // Show editor view in full screen
#ifdef QTOPIA_PHONE
    if ( widget_stack ) {
        widget_stack->setParent( 0 );
        widget_stack->showFullScreen();
    } else {
        image_ui->setParent(0);
        image_ui->showFullScreen();
    }
#else
    image_ui->setParent( 0 );
    image_ui->showFullScreen();
#endif
    navigator->setFocus();
#ifdef QTOPIA_PHONE
    if( !Qtopia::mousePreferred() ) {
        connect( region_selector, SIGNAL( pressed() ),
            this, SLOT( exitCurrentEditorState() ) );
    }
#endif
    editor_state = FULL_SCREEN;
}

void PhotoEditUI::setViewThumbnail()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::setViewThumbnail() being called when there is NO image selector.");
        return;
    }

#ifdef QTOPIA_PHONE
    // If image selector not in multi, change to single and update context menu
    if( image_selector->viewMode() != QImageDocumentSelector::Thumbnail )
    {
        image_selector->setViewMode( QImageDocumentSelector::Thumbnail );
    }
    toggleActions();

#else
    image_selector->setViewMode( QImageDocumentSelector::Thumbnail );
#endif
}

void PhotoEditUI::setViewSingle()
{
    if ( !image_selector ) {
        qWarning("PhotoEditUI::setViewSingle() being called when there is NO image selector.");
        return;
    }

#ifdef QTOPIA_PHONE
    // If image selector not in single, change to single and update context menu
    if( image_selector->viewMode() != QImageDocumentSelector::Single )
    {
        image_selector->setViewMode( QImageDocumentSelector::Single );
    }
    toggleActions();
#else
    image_selector->setViewMode( QImageDocumentSelector::Single );
#endif
}

void PhotoEditUI::launchPopupMenu( const QContent&, const QPoint&
#ifndef QTOPIA_PHONE
pos
#endif
)
{
#ifndef QTOPIA_PHONE
    selector_menu->popup( pos );
#endif
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
        slide_show->setSlideLength(
            slide_show_dialog->slideLength() );
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

void PhotoEditUI::exitCurrentUIState()
{
    switch( ui_state ) {
    case SELECTOR:
#ifdef QTOPIA_PHONE
        if ( !image_selector ) {
            qWarning("PhotoEditUI::exitCurrentUIState() being called for context requiring image selector.");
            break;
        }

        switch( image_selector->viewMode() ) {
        // If in single view
        case QImageDocumentSelector::Single:
            // If there are images, change to thumbnail and update context menu
            // Otherwise, close application
            if( image_selector->documents().count() ) {
                setViewThumbnail();
            } else {
                close_ok = true;
            }
            break;
        // If in thumbnail view, close application
        case QImageDocumentSelector::Thumbnail:
            close_ok = true;
            break;
        }
#endif
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
#ifdef QTOPIA_PHONE
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
                close_ok = true;
            }
        } else {
            enterSelector();
        }
        edit_canceled = false;
#else
        if( service_requested )
            sendValueSupplied();
        else
            saveChanges();
        enterSelector();
#endif
        break;
    }
}

void PhotoEditUI::exitCurrentEditorState()
{
    switch( editor_state ) {
    // If in view, no change
    case VIEW:
#ifdef QTOPIA_PHONE
        editor_state_changed = false;
#endif
        break;
    // If in full screen, return from full screen
    case FULL_SCREEN:
#ifdef QTOPIA_PHONE
        if( !Qtopia::mousePreferred() ) {
            disconnect( region_selector, SIGNAL( pressed() ),
                this, SLOT( exitCurrentEditorState() ) );
        }
#endif
        // Set editor central widget to editor view
#ifdef QTOPIA_PHONE
        if ( widget_stack ) {
            layout()->addWidget( widget_stack );
        } else {
            layout()->addWidget(image_ui);
        }
#else
        image_ui->setParent( editor_ui );

        editor_ui->setCentralWidget( image_ui );
        editor_ui->setFocus();
#endif

#ifdef QTOPIA_PHONE
        if( !Qtopia::mousePreferred() ) was_fullscreen = true;
#endif
        editor_state = VIEW;
#ifdef QTOPIA_PHONE
        editor_state_changed = true;
#endif
        break;
    // If in zoom, hide zoom control
    case ZOOM:
        zoom_slider->hide();
        // Show navigator
        navigator->show();
#ifdef QTOPIA_PHONE
        navigator->setFocus();
#endif
        editor_state = VIEW;
#ifdef QTOPIA_PHONE
        editor_state_changed = true;
#endif
        break;
    // If in brightness, hide brightness control
    case BRIGHTNESS:
        brightness_slider->hide();
        // Show navigator
        navigator->show();
#ifdef QTOPIA_PHONE
        navigator->setFocus();
#endif
        editor_state = VIEW;
#ifdef QTOPIA_PHONE
        editor_state_changed = true;
#endif
        break;
    // If in crop, disable region selector and show navigator
    case CROP:
        region_selector->setEnabled( false );
        region_selector->update();
        // Show navigator
        navigator->show();
        navigator->setFocus();
        editor_state = VIEW;
#ifdef QTOPIA_PHONE
        editor_state_changed = true;
#endif
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
#ifdef QTOPIA_PHONE
    edit_canceled = true;
    close();
#endif
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
    srv << image.file();
    srv << image.mimeTypes().at(0);
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

    // Lauch confirmation dialog
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

void PhotoEditUI::closeEvent( QCloseEvent* e )
{
#ifdef QTOPIA_PHONE
    close_ok = false;
    e->ignore();
    if( Qtopia::mousePreferred() ) {
        if( ui_state != EDITOR || ( editor_state == VIEW && !editor_state_changed ) || edit_canceled ) {
            exitCurrentUIState();
            if( close_ok ) e->accept();
        }
    } else {
        if( was_fullscreen ) was_fullscreen = false;
        else {
            if( ui_state == EDITOR && editor_state != VIEW )
                exitCurrentEditorState();
            else {
                exitCurrentUIState();
                if( close_ok )
                    e->accept();
            }
        }
    }
#else
    exitCurrentEditorState();
    exitCurrentUIState();
    // Reverse effects of only editor launch for fast load
    if( only_editor ) {
        if ( widget_stack ) {
            widget_stack->setCurrentIndex( widget_stack->indexOf( selector_ui ) );
        }
        editor_ui->addFileItems();
        only_editor = service_requested = false;
        service_image = QImage();
    }
    e->accept();
#endif

#ifdef QTOPIA_PHONE
    if( close_ok )
#endif
       qApp->quit();
}

#ifndef QTOPIA_PHONE
void PhotoEditUI::interruptCurrentState()
{
    // If editing, conclude editing
    if( ui_state == EDITOR ) {
        if( service_requested ) sendValueSupplied();
        else saveChanges();
    }
}
#endif

void PhotoEditUI::clearEditor()
{
    if ( navigator ) {
        navigator->hide();
    }
#ifndef QTOPIA_PHONE
    zoom_slider->hide();
    brightness_slider->hide();
#endif
    if ( image_ui ) {
        image_ui->setEnabled( false );
        image_ui->repaint();
    }
}

void PhotoEditUI::saveChanges()
{
    // If image was changed, prompt user to save changes
    if( image_processor->isChanged() && current_image.drmState() == QContent::Unprotected ) {
#ifndef QTOPIA_PHONE
        if( QMessageBox::information( this, tr( "Save Changes" ),
            tr( "<qt>Do you want to save your changes?</qt>" ),
            QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
#endif
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
                const char* format = image_io->format();
                QMessageBox::warning( this,
                    tr( "Saving %1" ).arg( format ),
                    tr( "<qt>Saving as %1 is not supported. "
                        "Using the default format instead.</qt>" )
                        .arg( format ) );
            }

            QImage image = image_processor->image();
            // Attempt to save changes
            bool saving = true;
            while( saving && !image_io->save( image, overwrite ) ) {
                 QMessageBox::warning(
                    this,
                    tr( "Save failed" ),
                    tr( "<qt>Your edits were not saved.</qt>" ) );
                 saving = false;
            }
#ifndef QTOPIA_PHONE
        }
#endif
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
    QImage img = image_processor->image
                    ( QSize( service_width, service_height ) );
    img.save(path,"JPEG");
    e << service_id << path;
    }
}

void PhotoEditUI::rightsExpired( const QDrmContent &content )
{
    Q_UNUSED( content );

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
    mParent->showMaximized();
    QTimer::singleShot( 0, mParent, SLOT( processShowCategory() ) );
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
