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

#ifndef PHOTOEDITUI_H
#define PHOTOEDITUI_H

#include "selector/selectorui.h"
#include "editor/editorui.h"
#include "editor/slider.h"
#include "editor/navigator.h"
#include "editor/regionselector.h"
#include "editor/imageui.h"
#include "editor/imageprocessor.h"
#include "editor/imageio.h"
#include "slideshow/slideshowdialog.h"
#include "slideshow/slideshowui.h"
#include "slideshow/slideshow.h"

#include <qimagedocumentselector.h>
#include <qsoftmenubar.h>
#include <qdrmcontent.h>
#include <qtopiaabstractservice.h>

#include <qstring.h>
#include <qslider.h>

#include <QStackedWidget>
#include <QMenu>

class QDSActionRequest;
class QResizeEvent;


class PhotoEditUI : public QWidget {
    Q_OBJECT
    friend class PhotoEditService;
public:
    PhotoEditUI( QWidget* parent, Qt::WFlags f );
    ~PhotoEditUI();

    // Process window deactivate events
    bool eventFilter( QObject*, QEvent* );

public slots:
    // Open image for editing
    void setDocument( const QString& lnk );

private slots:
    // Respond to service request
    void appMessage( const QString&, const QByteArray& );

    // Open given image for editing
    void processSetDocument();

    // Change category and show selector
    void processShowCategory();

    // Open given image for editing
    void processGetImage();

    // Toggle actions dependant of images in image selector
    void toggleActions();

    // Raise selector to top of widget stack
    void enterSelector();

    // Raise slide show to top of widget stack and start
    void enterSlideShow();

    // Raise editor to top of widget stack and load current image
    void enterEditor();

    // Show zoom control
    void enterZoom();

    // Show brightness control
    void enterBrightness();

    // Enable region selector and hide naviagtor
    void enterCrop();

    // Show editor view in full screen
    void enterFullScreen();

    // Change to single view in image selector
    void setViewSingle();

    // Change to multi view in image selector
    void setViewThumbnail();

    // Only Qtopia PDA
    // Launch selector popup menu
    void launchPopupMenu( const QContent&, const QPoint& );

    // Launch slide show dialog
    void launchSlideShowDialog();

    // Launch properties dialog
    void launchPropertiesDialog();

    // Move to previous UI state
    // Enable application to be closed if no previous state exists
    void exitCurrentUIState();

    // Move to previous editor state
    void exitCurrentEditorState();

    // Set zoom factor in image processor
    void setZoom( int );

    // Set brightness factor in image processor
    void setBrightness( int );

    // Open currently highlighted image in image selector for editing
    void editCurrentSelection();

    // Only Qtopia Phone
    // Ignore changes to image and exit from editor
    void cancelEdit();

    // Perform crop on current image using region from region selector
    void cropImage();

    // Send current image over IR link
    void beamImage();

    // Print current image
    void printImage();

    // Delete current image
    void deleteImage();

    // Show selector if image currently being edited is deleted
    void contentChanged( const QContentIdList &id, const QContent::ChangeType type );

    // Rights to the open image have expired so close.
    void rightsExpired( const QDrmContent &content );

    // Edit an image contained within the QDS request
    void editImage( const QDSActionRequest& request );

protected:
    // Move to previous state, close application if no previous state exists
    void closeEvent( QCloseEvent* );

    void resizeEvent(QResizeEvent *event);

private:

    void init(bool listMode);

#ifndef QTOPIA_PHONE
    // Interrupt and conclude current state
    void interruptCurrentState();
#endif

    // Hide editor controls, clear and show editor
    void clearEditor();

    // Prompt user to save changes to image if image was modified
    void saveChanges();

    // Send modified image back in qcop message
    void sendValueSupplied();

    enum { SELECTOR, SLIDE_SHOW, EDITOR } ui_state;

    enum { VIEW, FULL_SCREEN, ZOOM, BRIGHTNESS, CROP } editor_state;

    bool only_editor, service_requested;

#ifdef QTOPIA_PHONE
    bool was_fullscreen, edit_canceled, close_ok, editor_state_changed;
#endif

    QContent service_lnk;
    QCategoryFilter service_category;
    QString service_channel;
    QString service_id;
    int service_width, service_height;
    QImage service_image;

    QContent current_image;
    QDrmContent selector_image;

#ifdef QTOPIA_PHONE
    QMenu *selector_menu;
    QAction *separator_action, *properties_action, *beam_action, *print_action;
    QAction *delete_action, *edit_action, *slide_show_action;
#else
    SelectorUI *selector_ui;
    QMenu *selector_menu;
#endif
    QImageDocumentSelector *image_selector;

    EditorUI *editor_ui;
    RegionSelector *region_selector;
    Navigator *navigator;
    Slider *brightness_slider, *zoom_slider;

    ImageUI *image_ui;
    ImageProcessor *image_processor;
    ImageIO *image_io;

    SlideShowDialog *slide_show_dialog;
    SlideShowUI *slide_show_ui;
    SlideShow *slide_show;

    QStackedWidget *widget_stack;

    QDSActionRequest* currEditImageRequest;
};

class PhotoEditService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class PhotoEditUI;
private:
    PhotoEditService( PhotoEditUI *parent )
        : QtopiaAbstractService( "PhotoEdit", parent ),
        mParent( parent ) { publishAll(); }

public:
    ~PhotoEditService();

public slots:
    void showCategory( const QString& category );
    void editImage( const QDSActionRequest& request );

private:
    PhotoEditUI* mParent;
};

#endif
