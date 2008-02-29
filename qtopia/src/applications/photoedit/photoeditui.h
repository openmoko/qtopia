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

#include <qtopia/imageselector.h>
#include <qtopia/contextmenu.h>
#include <qtopia/applnk.h>

#include <qvbox.h>
#include <qstring.h>
#include <qwidgetstack.h>
#include <qslider.h>
#include <qpopupmenu.h>

class PhotoEditUI : public QVBox {
    Q_OBJECT
public:
    PhotoEditUI( QWidget* parent, const char* name, WFlags f );
    
    // Process window deactivate events
    bool eventFilter( QObject*, QEvent* );
    
public slots:
    // Open image for editing
    void setDocument( const QString& lnk );
    
private slots:
    // Respond to service request
    void appMessage( const QCString&, const QByteArray& );
    
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
    
    // Only Qtopia PDA
    // Launch selector popup menu
    void launchPopupMenu( const DocLnk&, const QPoint& );
    
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
    
    // Delete current image
    void deleteImage();
    
    // Show selector if image currently being edited is deleted
    void linkChanged( const QString& );
    
protected:
    // Move to previous state, close application if no previous state exists
    void closeEvent( QCloseEvent* );

private:
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
    
    DocLnk service_lnk;
    int service_category;
    QCString service_channel;
    QString service_id;
    int service_width, service_height;
    QImage service_image;
        
    DocLnk current_image;

#ifdef QTOPIA_PHONE
    ContextMenu *selector_menu;
    int separator_id;
#else
    SelectorUI *selector_ui;
    QPopupMenu *selector_menu;
#endif
    ImageSelector *image_selector;
    
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
    
    QWidgetStack *widget_stack;
};

#endif
