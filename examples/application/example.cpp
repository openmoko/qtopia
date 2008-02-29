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

#include "example.h"
#include <qpushbutton.h>
#include <qsoftmenubar.h>

/*
 *  Constructs a Example which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
Example::Example(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
{
    setupUi(this); // reads the .ui file to determine widgets and layout
    // Construct context menu, available to the user via Qtopia's soft menu bar.
    QMenu *menu = QSoftMenuBar::menuFor(this);
    Q_UNUSED(menu); // Normally, we would use "menu" to add more actions.
    // The context menu will not be active unless it has at least one action.
    // Tell Qtopia to provide the "Help" option, which will enable the user to
    // read the Help documentation for this application.
    QSoftMenuBar::setHelpEnabled(this,true);
    // To add further actions, see QWidget::insertAction(QAction *,QAction *)
}

/*
 *  Destroys the object and frees any allocated resources
 */
Example::~Example()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Every time the quit button is clicked, this slot will be invoked
 *  (see QMetaObject::connectSlotsByName()).
 */
void Example::on_QuitButton_clicked()
{
    close();
}

