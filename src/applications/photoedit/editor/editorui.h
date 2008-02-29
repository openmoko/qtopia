/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qmainwindow.h>

#ifndef QTOPIA_PHONE
class QAction;
class QMenu;
#endif

class EditorUI : public QMainWindow
{
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
#ifndef QTOPIA_PHONE
    // Open selected
    void open();
#else
    // Cancel selected
    void cancel();
#endif

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
#ifndef QTOPIA_PHONE
    QAction *open_action;
    QMenu *file_menu;
#endif
};

#endif
