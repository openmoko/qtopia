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

#ifndef EDITORUI_H
#define EDITORUI_H

#include <qsoftmenubar.h>

#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qaction.h>

class EditorUI : public QMainWindow {
    Q_OBJECT
public:
    EditorUI( QWidget* parent = 0, Qt::WFlags f = 0 );

#ifndef QTOPIA_PHONE
    // Add file items to the ui
    void addFileItems();

    // Remove file items from the ui
    void removeFileItems();
#endif

signals:
    // Only Qtopia PDA
    // Open selected
    void open();

    // Only Qtopia Phone
    void cancel();

    // Crop selected;
    void crop();

    // Brightness selected
    void brightness();

    // Rotate selected
    void rotate();

    // Zoom selected
    void zoom();

    // Fullscreen selected
    void fullScreen();

protected:
    void closeEvent( QCloseEvent * );
private:
    QAction *open_action;
#ifndef QTOPIA_PHONE
    QAction *brightness_action, *rotate_action, *crop_action;
    QAction *zoom_action, *fullscreen_action;
#endif

#ifdef QTOPIA_PHONE
    QMenu *context_menu;
#else
    QToolBar *toolbar;
    QMenuBar *menubar;
    QMenu *file_menu;
#endif
};

#endif
