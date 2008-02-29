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
#ifndef DATEBOOKGUI_H
#define DATEBOOKGUI_H

#include <qmainwindow.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

class QAction;
class QToolBar;

class DateBookGui : public QMainWindow
{
    Q_OBJECT
public:
    DateBookGui( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~DateBookGui();

protected:
    virtual void init();

    QWidget *parentWidget;
    QAction *actionDay, *actionWeek, *actionMonth, *actionNextView;
    QAction *actionNew, *actionEdit, *actionDelete,
            *actionBeam, *actionToday, *actionFind,
            *actionSettings, *actionBack, *actionPurge,
            *actionShowAll, *actionHideSome;
    QAction *actionCategory;
    QAction *actionAccounts;

    QToolBar *sub_bar;
    QToolBar *details_bar;

#ifdef QTOPIA_PHONE
    QMenu *contextMenu;
#endif
};

#endif
