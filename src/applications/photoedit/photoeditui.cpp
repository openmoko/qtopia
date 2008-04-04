/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "thumbnailmodel.h"
#include "imageviewer.h"
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qdocumentproperties.h>
#include <qtopiaipcenvelope.h>
#include <qtopiasendvia.h>
#include <qtopiaservices.h>
#include <qcontent.h>
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
#include <QListView>
#include <QContentFilterDialog>
#include <QDesktopWidget>

#include <cmath>

PhotoEditUI::PhotoEditUI( QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f )
    , service_requested( false )
    , is_fullscreen( false )
    , was_fullscreen( false )
    , edit_canceled( false )
    , separator_action(0)
    , properties_action(0)
    , beam_action(0)
    , print_action(0)
    , delete_action(0)
    , edit_action(0)
    , slide_show_action(0)
    , fullscreen_action(0)
    , viewer_edit_action(0)
    , image_viewer(0)
    , selector_view(0)
    , type_label(0)
    , category_label(0)
    , selector_widget(0)
    , region_selector(0)
    , navigator(0)
    , brightness_slider(0)
    , zoom_slider(0)
    , brightness_widget(0)
    , zoom_widget(0)
    , image_ui(0)
    , image_processor(0)
    , image_io(0)
    , editor_stack(0)
    , slide_show_dialog(0)
    , slide_show_ui(0)
    , slide_show(0)
    , widget_stack(0)
    , type_dialog( 0 )
    , category_dialog( 0 )
    , currEditImageRequest(0)
    , list_init_timer_id(-1)
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

    list_init_timer_id = startTimer( 0 );
}

PhotoEditUI::~PhotoEditUI()
{
    delete currEditImageRequest;
    currEditImageRequest = 0;
}

bool PhotoEditUI::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

    if (event->type() == QEvent::MouseButtonPress) {
        exitCurrentEditorState();

        return true;
    }

    return false;
}


ImageViewer *PhotoEditUI::imageViewer()
{
    if( !image_viewer )
    {
        image_viewer = new ImageViewer;

        image_viewer->setScaleMode( ImageViewer::ScaleToFit );

        connect( image_viewer, SIGNAL(imageInvalidated()), this, SLOT(exitCurrentUIState()) );

        QMenu *viewer_menu = QSoftMenuBar::menuFor( image_viewer );

        viewer_edit_action = viewer_menu->addAction( QIcon( ":icon/edit" ), tr( "Edit" ), this, SLOT(editCurrentSelection()) );
        viewer_menu->addAction( QIcon( ":icon/info" ), tr( "Properties" ), this, SLOT(launchPropertiesDialog()) );
        viewer_menu->addAction( QIcon( ":icon/beam" ), tr( "Send" ), this, SLOT(beamImage()) );
        viewer_menu->addAction( QIcon( ":icon/print" ), tr( "Print" ), this, SLOT(printImage()) );
        viewer_menu->addAction( QIcon( ":icon/trash" ), tr( "Delete" ), this, SLOT(deleteImage()) );

        connect( viewer_menu, SIGNAL(aboutToShow()), this, SLOT(viewerMenuAboutToShow()) );

        QSoftMenuBar::setLabel( image_viewer, Qt::Key_Select, QSoftMenuBar::NoLabel );

        widget_stack->addWidget( image_viewer );
    }

    return image_viewer;
}

QWidget *PhotoEditUI::selectorWidget()
{
    if( !selector_widget )
    {
        QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

        QStringList types = settings.value( QLatin1String( "Types" ) ).toString().split( QLatin1Char( ';' ), QString::SkipEmptyParts );
        QStringList categories = settings.value( QLatin1String( "Categories" ) ).toString().split( QLatin1Char( ';' ), QString::SkipEmptyParts );

        foreach( QString type, types )
            type_filter |= QContentFilter::mimeType( type );
        foreach( QString category, categories )
            category_filter |= QContentFilter::category( category );

        QString typeLabel;
        if( types.count() == 1 )
            typeLabel = tr("Type: %1").arg( types.first() );
        else if( types.count() > 1 )
            typeLabel = tr("Type: %1").arg( tr( "(Multi)" ) );

        QString categoryLabel;
        if( categories.count() == 1 )
        {
            if( categories.first() == QLatin1String( "Unfiled" ) )
            {
                categoryLabel = tr("Category: %1").arg( tr( "Unfiled" ) );
            }
            else
            {
                QCategoryManager manager;

                categoryLabel = tr("Category: %1").arg( manager.label( categories.first() ) );
            }
        }
        else if( categories.count() > 1 )
            categoryLabel = tr("Category: %1").arg( tr( "(Multi)" ) );


        image_set = new QContentSet( QContentSet::Asynchronous, this );

        image_set->setCriteria(
                QContentFilter( QContent::Document )
            & category_filter
            & (type_filter.isValid() ? type_filter : QContentFilter::mimeType( QLatin1String( "image/*" ) )) );


        int iconSize = (36 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;

        image_model = new ThumbnailContentSetModel( image_set, this );
        image_model->setThumbnailSize( QSize( iconSize, iconSize ) );

        connect( image_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                 this,          SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );

        connect( image_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                 this,          SLOT(rowsInserted(QModelIndex,int,int)) );

        image_model->setView( selector_view = new ContentThumbnailView );


        selector_view->setModel( image_model );
        selector_view->setViewMode( QListView::IconMode );
        selector_view->setIconSize( QSize( iconSize, iconSize ) );
        selector_view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        selector_view->setFrameStyle( QFrame::NoFrame );
        selector_view->setResizeMode( QListView::Fixed );
        selector_view->setSelectionMode( QAbstractItemView::SingleSelection );
        selector_view->setSelectionBehavior( QAbstractItemView::SelectItems );
        selector_view->setUniformItemSizes( true );
        selector_view->setItemDelegate( new ContentThumbnailDelegate( selector_view ) );


        QMenu *selector_menu = QSoftMenuBar::menuFor( selector_view );

        slide_show_action  = selector_menu->addAction( QIcon( ":icon/slideshow" ), tr( "Slide Show..." ), this, SLOT(launchSlideShowDialog()) );

        selector_menu->addSeparator();

        edit_action = selector_menu->addAction( QIcon( ":icon/edit" ), tr( "Edit" ), this, SLOT(editCurrentSelection()) );
        properties_action = selector_menu->addAction( QIcon( ":icon/info" ), tr( "Properties" ), this, SLOT(launchPropertiesDialog()) );
        beam_action = selector_menu->addAction( QIcon( ":icon/beam" ), tr( "Send" ), this, SLOT(beamImage()) );
        print_action = selector_menu->addAction( QIcon( ":icon/print" ), tr( "Print" ), this, SLOT(printImage()) );
        delete_action = selector_menu->addAction( QIcon( ":icon/trash" ), tr( "Delete" ), this, SLOT(deleteImage()) );

        slide_show_action->setVisible( false );
        edit_action->setVisible( false );
        beam_action->setVisible( false );
        print_action->setVisible( false );
        delete_action->setVisible( false );
        properties_action->setVisible( false );

        separator_action = selector_menu->addSeparator();

        selector_menu->addAction( tr( "View Type..." ), this, SLOT(selectType()) );
        selector_menu->addAction( QIcon( QLatin1String( ":icon/viewcategory" ) ), tr( "View Category..." ), this, SLOT(selectCategory()) );

        connect( selector_menu, SIGNAL(aboutToShow()), this, SLOT(selectorMenuAboutToShow()) );

        connect( selector_view, SIGNAL(activated(QModelIndex)),
                 this, SLOT(imageSelected(QModelIndex)) );

        type_label = new QLabel( typeLabel );
        type_label->setVisible( !typeLabel.isEmpty() );

        category_label = new QLabel( categoryLabel );
        category_label->setVisible( !categoryLabel.isEmpty() );

        QVBoxLayout *layout = new QVBoxLayout;

        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( selector_view );
        layout->addWidget( type_label );
        layout->addWidget( category_label );

        selector_widget = new QWidget;
        selector_widget->setLayout( layout );
        selector_widget->setFocusProxy( selector_view );

        widget_stack->addWidget( selector_widget );
    }

    return selector_widget;
}

ImageUI *PhotoEditUI::imageEditor()
{
    if( !image_io )
    {
        // Construct image io
        image_io = new ImageIO( this );

        // Construct image processor
        image_processor = new ImageProcessor( image_io, this );

        // Construct image_ui or editor ui
        image_ui = new ImageUI(image_processor);

        image_ui->setObjectName( QLatin1String( "editmode" ) );

        editor_stack = new QStackedLayout;

        // Construct navigator
        navigator = new Navigator( image_ui );

        editor_stack->addWidget( navigator );

        // Construct region selector
        region_selector = new RegionSelector( image_ui );
        region_selector->setEnabled( true );
        region_selector->setObjectName( QLatin1String( "dimensions" ) );

        if( Qtopia::mousePreferred() )
            QSoftMenuBar::menuFor( region_selector );

        editor_stack->addWidget( region_selector );

        connect( region_selector, SIGNAL(selected()),
            this, SLOT(cropImage()) );
        connect( region_selector, SIGNAL(selected()),
            this, SLOT(exitCurrentEditorState()) );

        // Construct brightness control
        brightness_slider = new Slider( -70, 70, 0, 0, 0 );

        connect( brightness_slider, SIGNAL(selected()),
            this, SLOT(exitCurrentEditorState()) );
        connect( brightness_slider, SIGNAL(valueChanged(int)),
            this, SLOT(setBrightness(int)) );


        QVBoxLayout *brightness_layout = new QVBoxLayout;
        brightness_layout->addStretch();
        brightness_layout->addWidget( brightness_slider );

        brightness_widget = new QWidget;
        brightness_widget->setLayout( brightness_layout );
        brightness_widget->setObjectName( QLatin1String( "brightness" ) );
        brightness_widget->installEventFilter(this);

        editor_stack->addWidget( brightness_widget );

        // Construct zoom control
        zoom_slider = new Slider( 100, 200, 10, 0, 0 );
        zoom_slider->setSingleStep( 5 );

        connect( zoom_slider, SIGNAL(selected()),
            this, SLOT(exitCurrentEditorState()) );
        connect( zoom_slider, SIGNAL(valueChanged(int)),
            this, SLOT(setZoom(int)) );

        QVBoxLayout *zoom_layout = new QVBoxLayout;
        zoom_layout->addStretch();
        zoom_layout->addWidget( zoom_slider );

        zoom_widget = new QWidget;
        zoom_widget->setLayout( zoom_layout );
        zoom_widget->installEventFilter(this);

        editor_stack->addWidget( zoom_widget );

        image_ui->setLayout( editor_stack );

        // Clear context bar
        QSoftMenuBar::setLabel( image_ui, Qt::Key_Select, QLatin1String( ":icon/view" ), tr( "Zoom" ) );

        // Construct context menu for image ui
        QMenu *context_menu = QSoftMenuBar::menuFor( image_ui );
        QSoftMenuBar::setHelpEnabled( image_ui, true );

        context_menu->addAction( QIcon( ":icon/cut" ), tr( "Crop" ), this, SLOT(enterCrop()) );
        context_menu->addAction( QIcon( ":icon/color" ), tr( "Brightness" ), this, SLOT(enterBrightness()) );
        QAction *rotate_action = context_menu->addAction( QIcon( ":icon/rotate" ), tr( "Rotate" ), image_processor, SLOT(rotate()) );
        connect( rotate_action, SIGNAL(triggered()), this, SLOT(exitCurrentEditorState()) );
        context_menu->addSeparator();
        context_menu->addAction( QIcon( ":icon/view" ), tr( "Zoom" ), this, SLOT(enterZoom()) );

        if( !Qtopia::mousePreferred() ) {
            fullscreen_action = context_menu->addAction( QIcon( ":icon/fullscreen" ), tr( "Full Screen" ), this, SLOT(enterFullScreen()) );
        }
        context_menu->addSeparator();
        context_menu->addAction( QIcon( ":icon/cancel" ), tr( "Cancel" ), this, SLOT(cancelEdit()) );

        widget_stack->addWidget(image_ui);
    }

    return image_ui;
}

void PhotoEditUI::timerEvent( QTimerEvent *event )
{
    if( event->timerId() == list_init_timer_id )
    {
        killTimer( list_init_timer_id );

        list_init_timer_id = -1;

        QWidget *selector = selectorWidget();

        widget_stack->setCurrentWidget( selector );

        navigation_stack.append( selector );

        selector->setFocus();
    }
}

bool PhotoEditUI::event( QEvent *event )
{
    if( event->type() == QEvent::WindowDeactivate && is_fullscreen )
    {
        lower();
    }
    else if( event->type() == QEvent::WindowActivate && is_fullscreen )
    {
        QString title = windowTitle();

        setWindowTitle( QLatin1String( "_allow_on_top_" ) );

        raise();

        setWindowTitle( title );
    }

     return QWidget::event( event );
}

void PhotoEditUI::setDocument( const QString& lnk )
{
    if( list_init_timer_id != -1 )
    {
        killTimer( list_init_timer_id );

        list_init_timer_id = -1;
    }

    viewImage( QContent( lnk, false ) );
}

void PhotoEditUI::editImage( const QDSActionRequest& request )
{
    if( list_init_timer_id != -1 )
    {
        killTimer( list_init_timer_id );

        list_init_timer_id = -1;
    }
    else
        hide();

    showMaximized();
    currEditImageRequest = new QDSActionRequest( request );
    QDataStream stream( currEditImageRequest->requestData().toIODevice() );
    QPixmap orig;
    stream >> orig;
    service_image = orig.toImage();
    clearEditor();
    service_requested = true;

    if( !navigation_stack.isEmpty() && navigation_stack.last() == image_viewer )
    {
        current_image = image_viewer->content();
        image_viewer->setContent( QContent() );
    }

    enterEditor();
}

void PhotoEditUI::imageSelected( const QModelIndex &index )
{
    viewImage( qvariant_cast< QContent >( index.data( Qt::UserRole + 1 ) ) );
}

void PhotoEditUI::viewImage( const QContent &content )
{
    ImageViewer *viewer = imageViewer();

    viewer->setContent( content );

    widget_stack->setCurrentWidget( viewer );

    navigation_stack.append( viewer );
}

void PhotoEditUI::selectType()
{
    if( !type_dialog )
    {
        QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

        QStringList types = settings.value( QLatin1String( "Types" ) ).toString().split( QLatin1Char( ';' ), QString::SkipEmptyParts );

        QContentFilterModel::Template typePage;

        typePage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        typePage.addList( QContentFilter::MimeType, QString(), types );

        type_dialog = new QContentFilterDialog( typePage, this );

        type_dialog->setWindowTitle( tr("View Type") );
        type_dialog->setFilter( QContentFilter( QContent::Document ) & QContentFilter::mimeType( QLatin1String( "image/*" ) ) );
        type_dialog->setObjectName( QLatin1String( "documents-type" ) );
    }

    QtopiaApplication::execDialog( type_dialog );

    type_filter = type_dialog->checkedFilter();

    image_set->setCriteria(
            QContentFilter( QContent::Document )
          & category_filter
          & (type_filter.isValid() ? type_filter : QContentFilter::mimeType( QLatin1String( "image/*" ) )) );

    QString label = type_dialog->checkedLabel();

    if( !type_filter.isValid() || label.isEmpty() )
    {
        type_label->setVisible( false );
    }
    else
    {
        type_label->setText( tr("Type: %1").arg( label ) );
        type_label->setVisible( true );
    }

    QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

    settings.setValue( QLatin1String( "Types" ), type_filter.arguments( QContentFilter::MimeType ).join( QLatin1String( ";" ) ) );
}

void PhotoEditUI::selectCategory()
{
    if( !category_dialog )
    {
        QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

        QStringList categories = category_filter.arguments( QContentFilter::Category );

        QContentFilterModel::Template categoryPage;

        categoryPage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        categoryPage.addList( QContentFilter::Category, QString(), categories );
        categoryPage.addList( QContentFilter::Category, QLatin1String( "Documents" ), categories );

        category_dialog = new QContentFilterDialog( categoryPage, this );

        category_dialog->setWindowTitle( tr("View Category") );
        category_dialog->setFilter( QContentFilter( QContent::Document ) & QContentFilter::mimeType( QLatin1String( "image/*" ) ) );
        category_dialog->setObjectName( QLatin1String( "documents-category" ) );
    }

    QtopiaApplication::execDialog( category_dialog );

    category_filter = category_dialog->checkedFilter();

    image_set->setCriteria(
            QContentFilter( QContent::Document )
          & category_filter
          & (type_filter.isValid() ? type_filter : QContentFilter::mimeType( QLatin1String( "image/*" ) )) );

    QString label = category_dialog->checkedLabel();

    if( !category_filter.isValid() || label.isEmpty() )
    {
        category_label->setVisible( false );
    }
    else
    {
        category_label->setText( tr("Category: %1").arg( label ) );
        category_label->setVisible( true );
    }

    QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

    settings.setValue( QLatin1String( "Categories" ), category_filter.arguments( QContentFilter::Category ).join( QLatin1String( ";" ) ) );
}

void PhotoEditUI::viewerMenuAboutToShow()
{
    viewer_edit_action->setEnabled( image_viewer->content().drmState() != QContent::Protected );
}

void PhotoEditUI::selectorMenuAboutToShow()
{
    edit_action->setEnabled( qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) ).drmState() != QContent::Protected );
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

void PhotoEditUI::enterSlideShow()
{
    if ( !selector_view ) {
        qWarning("PhotoEditUI::enterSlideShow() being called when there is NO image selector.");
        return;
    }

    // Set slide show collection from currently visible collection in selector
    slide_show->setCollection( *image_set );
    // Set first image in slideshow to currently selected image in selector
    slide_show->setFirstImage( qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) ) );

    widget_stack->setCurrentWidget( slide_show_ui );

    navigation_stack.append( slide_show_ui );

    // Start slideshow
    slide_show->start();
    QtopiaApplication::setPowerConstraint( QtopiaApplication::Disable );
}

void PhotoEditUI::enterEditor()
{
#define LIMIT( X, Y, Z ) ( (X) > (Y) ? (X) > (Z) ? (Z) : (X) : (Y) )
#define REDUCTION_RATIO( dw, dh, sw, sh ) \
    ( (dw)*(sh) > (dh)*(sw) ? (double)(dh)/(double)(sh) : \
    (double)(dw)/(double)(sw) )

    ImageUI *editor = imageEditor();

    editor->setEnabled( false );

    // Raise editor to top of widget stack
    widget_stack->setCurrentWidget( editor );

    navigation_stack.append( editor );

    // Update image io with current image
    ImageIO::Status status;
    if( service_requested && !service_image.isNull() ) {
        status = image_io->load( service_image );
    } else if ( !current_image.isValid() ) {
        // I believe this is because it is possible to get in here before
        // an image has been established.
        status = ImageIO::LOAD_ERROR;
    } else {
        status = image_io->load(current_image);
    }

    switch( status ) {
    case ImageIO::REDUCED_SIZE:
        {
            QMessageBox mb(
                    QMessageBox::Information,
                    tr("Scaled Image"),
                    tr("<qt>Unable to load complete image.  A scaled copy has been opened instead.</qt>"),
                    QMessageBox::Ok);

            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();
        }
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
            editor_stack->setCurrentWidget( navigator );
        }
        break;

    case ImageIO::SIZE_ERROR:
    case ImageIO::LOAD_ERROR:
        {
            QMessageBox mb(
                    QMessageBox::Warning,
                    tr("Load Error"),
                    tr("<qt>Unable to load image.</qt>"),
                    QMessageBox::Ok);
            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();

            navigation_stack.removeLast();
            if( !navigation_stack.isEmpty() )
            {
                if( navigation_stack.last() == image_viewer )
                    image_viewer->setContent( current_image );

                widget_stack->setCurrentWidget( navigation_stack.last() );
            }
            else
                close();
            break;
        }
    case ImageIO::DEPTH_ERROR:
        {
            QMessageBox mb(
                    QMessageBox::Warning,
                    tr("Depth Error"),
                    tr("<qt>Image depth is not supported.</qt>"),
                    QMessageBox::Ok);
            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();

            navigation_stack.removeLast();
            if( !navigation_stack.isEmpty() )
            {
                if( navigation_stack.last() == image_viewer )
                    image_viewer->setContent( current_image );

                widget_stack->setCurrentWidget( navigation_stack.last() );
            }
            else
                close();
            break;
        }
    }
}

void PhotoEditUI::enterZoom()
{
    editor_stack->setCurrentWidget( zoom_widget );
}

void PhotoEditUI::enterBrightness()
{
    editor_stack->setCurrentWidget( brightness_widget );
}

void PhotoEditUI::enterCrop()
{
    // Enable selection in region selector
    editor_stack->setCurrentWidget( region_selector );

    region_selector->reset();
}

void PhotoEditUI::enterFullScreen()
{
    // Show editor view in full screen
    QString title = windowTitle();

    setWindowTitle( QLatin1String( "_allow_on_top_" ) );
    showFullScreen();
    raise();

    setWindowTitle( title );

    fullscreen_action->setVisible( false );

    is_fullscreen = true;
}

void PhotoEditUI::setViewThumbnail()
{
    if ( !selector_view ) {
        qWarning("PhotoEditUI::setViewThumbnail() being called when there is NO image selector.");
        return;
    }

    // If image selector not in multi, change to single and update context menu
    widget_stack->setCurrentWidget( selector_widget );
}

void PhotoEditUI::setViewSingle()
{
    if ( !selector_view ) {
        qWarning("PhotoEditUI::setViewSingle() being called when there is NO image selector.");
        return;
    }

    // If image selector not in single, change to single and update context menu
    widget_stack->setCurrentWidget( image_viewer );
}

void PhotoEditUI::launchSlideShowDialog()
{
    if ( !slide_show_dialog ) {
        slide_show_dialog = new SlideShowDialog( this );
        slide_show_dialog->setObjectName( QLatin1String( "slideshow" ) );
    }

    // If slide show dialog accepted, start slideshow
    if( QtopiaApplication::execDialog( slide_show_dialog, true ) ) {
        if ( !slide_show ) {
            slide_show = new SlideShow( this );
            slide_show_ui = new SlideShowUI(this);
            slide_show_ui->setWindowTitle( windowTitle() );

            widget_stack->addWidget( slide_show_ui );

            // Update image when slide show has changed
            connect( slide_show, SIGNAL(changed(QContent)),
                    slide_show_ui, SLOT(setImage(QContent)) );
            // Stop slide show when slide show ui pressed
            connect( slide_show_ui, SIGNAL(pressed()), slide_show, SLOT(stop()) );
            // Show selector when slide show has stopped
            connect( slide_show, SIGNAL(stopped()), this, SLOT(exitCurrentUIState()) );
        }
        // Set slide show options
        slide_show_ui->setDisplayName( slide_show_dialog->isDisplayName() );
        slide_show->setSlideLength( slide_show_dialog->slideLength() );
        slide_show->setLoopThrough( slide_show_dialog->isLoopThrough() );
        enterSlideShow();
    }
}

void PhotoEditUI::launchPropertiesDialog()
{
    QContent image;

    if( widget_stack->currentWidget() == image_viewer )
        image = image_viewer->content();
    else if( widget_stack->currentWidget() == selector_widget )
        image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );
    else
        return;

    QDocumentPropertiesDialog dialog( image );
    dialog.setObjectName( QLatin1String( "properties" ) );
    // Launch properties dialog with current image
    QtopiaApplication::execDialog( &dialog );
}

bool PhotoEditUI::exitCurrentUIState()
{
    if( navigation_stack.isEmpty() )
        return true;

    navigation_stack.removeLast();

    if( widget_stack->currentWidget() == image_ui )
    {
        if( service_requested ) {
            if( !edit_canceled ) {
                sendValueSupplied();
            }
            setWindowState( windowState() | Qt::WindowMinimized );
        } else if( !edit_canceled ) {
            saveChanges();
        }
        edit_canceled = false;

        if( !navigation_stack.isEmpty() && navigation_stack.last() == image_viewer )
            image_viewer->setContent( current_image );
    }
    else if( widget_stack->currentWidget() == image_viewer )
    {
        image_viewer->setContent( QContent() );
    }
    else if( widget_stack->currentWidget() == slide_show_ui )
    {
        QtopiaApplication::setPowerConstraint( QtopiaApplication::Enable );
    }

    if( !navigation_stack.isEmpty() )
    {
        widget_stack->setCurrentWidget( navigation_stack.last() );

        return false;
    }
    else
    {
        return true;
    }
}

void PhotoEditUI::exitCurrentEditorState()
{
    if( editor_stack->currentWidget() == navigator ) {
        // If in full screen, return from full screen
        if( is_fullscreen ) {
            is_fullscreen = false;
            fullscreen_action->setVisible( true );

            QTimer::singleShot( 0, this, SLOT(showMaximized()) );
        // If in view, no change
        }
    } else {
        if( editor_stack->currentWidget() == region_selector ) {
            region_selector->update();
        }

        editor_stack->setCurrentWidget( navigator );
    }
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
    if( widget_stack->currentWidget() == image_viewer )
    {
        current_image = image_viewer->content();

        image_viewer->setContent( QContent() );

        enterEditor();
    }
    else if( widget_stack->currentWidget() == selector_widget )
    {
        current_image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );

        enterEditor();
    }
}

void PhotoEditUI::cancelEdit()
{
    if( is_fullscreen ) {
        exitCurrentEditorState();
    } else {
        edit_canceled = true;

        if( exitCurrentUIState() )
            close();
    }
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
    // Send current image over IR link
    QContent image;

    if( widget_stack->currentWidget() == image_viewer )
        image = image_viewer->content();
    else if( widget_stack->currentWidget() == selector_widget )
        image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );
    else
        return;

    QtopiaSendVia::sendFile(this, image);
}

void PhotoEditUI::printImage()
{
    QContent image;

    if( widget_stack->currentWidget() == image_viewer )
        image = image_viewer->content();
    else if( widget_stack->currentWidget() == selector_widget )
        image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );
    else
        return;

    QtopiaServiceRequest srv( "Print", "print(QString,QString)" );
    srv << image.fileName();
    srv << (image.mimeTypes().count() ? image.mimeTypes().at(0) : QString());
    srv.send();
}

void PhotoEditUI::deleteImage()
{
    // Retrieve currently highlighted image from selector
    QContent image;

    if( widget_stack->currentWidget() == image_viewer )
        image = image_viewer->content();
    else if( widget_stack->currentWidget() == selector_widget )
        image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );
    else
        return;

    // Launch confirmation dialog
    // If deletion confirmed, delete image
    QMessageBox mb(
            QMessageBox::Warning,
            tr("Delete"),
            tr("<qt>Are you sure you want to delete %1?</qt>"," %1 = file name").arg(image.name()),
            QMessageBox::Yes | QMessageBox::No);

    mb.setEscapeButton(QMessageBox::No);

    if( mb.exec() == QMessageBox::Yes ) {
        image.removeFiles();

        if ( widget_stack->currentWidget() == image_viewer && exitCurrentUIState() ) {
            close();
        }
    }
}

void PhotoEditUI::contentChanged( const QContentIdList &idList, const QContent::ChangeType type )
{
    // If content is current image and has been deleted, show selector
    if ( QContent::Removed == type && widget_stack->currentWidget() == image_viewer && idList.contains(current_image.id()) ) {
        navigation_stack.removeLast();
        if( !navigation_stack.isEmpty() )
            widget_stack->setCurrentWidget( navigation_stack.last() );
        else
            close();
    }
}

void PhotoEditUI::rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
{
    if( end - start + 1 == selector_view->model()->rowCount( parent ) )
    {
        QSoftMenuBar::setLabel( selector_view, Qt::Key_Select, QSoftMenuBar::NoLabel );

        slide_show_action->setVisible( false );
        edit_action->setVisible( false );
        beam_action->setVisible( false );
        print_action->setVisible( false );
        delete_action->setVisible( false );
        properties_action->setVisible( false );
    }
}

void PhotoEditUI::rowsInserted( const QModelIndex &parent, int start, int end )
{
    if( end - start + 1 == selector_view->model()->rowCount( parent ) )
    {
        QSoftMenuBar::setLabel( selector_view, Qt::Key_Select, QSoftMenuBar::View );

        slide_show_action->setVisible( true );
        edit_action->setVisible( true );
        beam_action->setVisible( true );
        print_action->setVisible( true );
        delete_action->setVisible( true );
        properties_action->setVisible( true );
    }
}

void PhotoEditUI::keyPressEvent( QKeyEvent *e )
{
    if( e->key() == Qt::Key_Back )
    {
        if( widget_stack->currentWidget() == image_ui && editor_stack->currentWidget() != navigator || is_fullscreen )
        {
            exitCurrentEditorState();
        }
        else if( exitCurrentUIState() )
        {
            close();
        }
        e->accept();
    }
    else if( e->key() == Qt::Key_Select && widget_stack->currentWidget() == image_ui && editor_stack->currentWidget() == navigator )
    {
        enterZoom();

        e->accept();
    }
    else if( selector_view && widget_stack->currentWidget() == image_viewer && (
             e->key() == Qt::Key_Left  ||
             e->key() == Qt::Key_Right ||
             e->key() == Qt::Key_Up    ||
             e->key() == Qt::Key_Down ) )
    {
        QModelIndex nextIndex;
        QModelIndex currentIndex = selector_view->currentIndex();

        if( e->key() == (QApplication::keyboardInputDirection() == Qt::LeftToRight ? Qt::Key_Left : Qt::Key_Right) ||
            e->key() == Qt::Key_Down )
            nextIndex = currentIndex.sibling( currentIndex.row() + 1, currentIndex.column() );
        else
            nextIndex = currentIndex.sibling( currentIndex.row() - 1, currentIndex.column() );

        if( nextIndex.isValid() )
        {
            selector_view->setCurrentIndex( nextIndex );
            selector_view->scrollTo( nextIndex );

            image_viewer->setContent( qvariant_cast< QContent >( nextIndex.data( Qt::UserRole + 1 ) ) );

            e->accept();
        }
    }
    else
        QWidget::keyPressEvent( e );
}

void PhotoEditUI::clearEditor()
{
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
                QMessageBox mb(
                        QMessageBox::Information,
                        tr("Read-Only File"),
                        tr("<qt>Saving a copy of the read-only file.</qt>"),
                        QMessageBox::Ok);
                mb.setEscapeButton(QMessageBox::Ok);

                mb.exec();
            } else {
                QMessageBox mb(
                    QMessageBox::Question,
                        tr("Save Changes "),
                        tr("<qt>Do you want to overwrite the original?</qt>"),
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Discard);

                mb.setEscapeButton(QMessageBox::Discard);

                switch(mb.exec()) {
                case QMessageBox::Yes:
                    overwrite = true;
                    break;
                case QMessageBox::Discard:
                    return;
                }
            }
        } else {
            QByteArray format = image_io->format();

            QMessageBox mb(
                    QMessageBox::Warning,
                    tr( "Saving %1" ).arg(format.constData()),
                    tr("<qt>Saving as %1 is not supported. Using the default format instead.</qt>"),
                    QMessageBox::Ok);

            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();
        }

        QImage image = image_processor->image();
        // Attempt to save changes

        QContent content = image_io->save( image, overwrite );

        if ( content.isNull() ) {
            QMessageBox mb(
                    QMessageBox::Warning,
                    tr("Save failed"),
                    tr("<qt>Your edits were not saved.</qt>"),
                    QMessageBox::Ok);

            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();
        } else {
            current_image = content;

            if ( !overwrite ) {
                QDocumentPropertiesDialog *properties = new QDocumentPropertiesDialog( current_image, this );

                if ( QtopiaApplication::execDialog( properties ) ) {
                    current_image = properties->document();
                }

                delete properties;
            }
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
    mParent->hide();
    mParent->showMaximized();

    while( !mParent->exitCurrentUIState() );

    if( mParent->category_dialog )
    {
        delete mParent->category_dialog;

        mParent->category_dialog = 0;
    }

    QWidget *selector = mParent->selectorWidget();

    mParent->navigation_stack.append( selector );

    mParent->widget_stack->setCurrentWidget( selector );

    mParent->category_filter = QCategoryFilter(category);

    mParent->image_set->setCriteria(
            QContentFilter( QContent::Document )
            & mParent->category_filter
            & (mParent->type_filter.isValid() ? mParent->type_filter : QContentFilter::mimeType( QLatin1String( "image/*" ) )) );
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
