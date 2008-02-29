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

#ifndef SELECTORUI_H
#define SELECTORUI_H

#include <qmainwindow.h>
#include <qaction.h>

class SelectorUI : public QMainWindow {
    Q_OBJECT
public:
    SelectorUI( QWidget* parent = 0, Qt::WFlags f = 0 );

    // Enable menu items if true
    void setEnabled( bool );

signals:
    // Edit selected
    void edit();

    // Delete selected
    void remove();

    // Beam selected
    void beam();

    // Properties selected
    void properties();

    // Slide show selected
    void slideShow();

private:
    QAction *edit_action, *delete_action, *beam_action, *properties_action;
    QAction *slide_show_action;
};

#endif
