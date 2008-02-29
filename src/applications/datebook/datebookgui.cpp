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

#include "datebookgui.h"

#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>
#include <QDesktopWidget>

#include <qtopiasendvia.h>

DateBookGui::DateBookGui( QWidget *parent, Qt::WFlags f )
    : QMainWindow( parent, f ),
    actionFind( 0 ),
    sub_bar( 0 ),
    details_bar( 0 )
{
}

DateBookGui::~DateBookGui()
{
}

void DateBookGui::init()
{
    parentWidget = this;

    setWindowTitle(tr("Calendar"));
    setWindowIcon(QPixmap(":image/DateBook"));

    //setToolBarsMovable( false );
    setBackgroundRole(QPalette::Button);

    // Create the actions

    actionNew = new QAction(QIcon(":icon/new"), tr("New"), parentWidget);
    actionNew->setWhatsThis(tr("Create a new event"));
    connect(actionNew, SIGNAL(triggered()), this, SLOT(newAppointment()));

    actionEdit = new QAction(QIcon(":icon/edit"), tr("Edit"), parentWidget);
    actionEdit->setWhatsThis(tr("Edit the selected event"));
    connect(actionEdit, SIGNAL(triggered()), this, SLOT(editCurrentOccurrence()));

    actionDelete = new QAction(QIcon(":icon/trash"), tr("Delete"), parentWidget);
    actionDelete->setWhatsThis(tr("Delete the selected event"));
    connect(actionDelete, SIGNAL(triggered()), this, SLOT(removeCurrentOccurrence()));

    if (QtopiaSendVia::isDataSupported("text/x-vcalendar")) {
        actionBeam = new QAction(QIcon(":icon/beam"), tr("Send"), parentWidget);
        actionBeam->setWhatsThis(tr("Beam the selected event"));
        connect(actionBeam, SIGNAL(triggered()), this, SLOT(beamCurrentAppointment()));
    }

    actionShowAll = new QAction(tr("Show All Events"), parentWidget);
    actionShowAll->setWhatsThis(tr("Show the all day events that are currently hidden"));
    connect(actionShowAll, SIGNAL(triggered()), this, SLOT(unfoldAllDay()));

    actionHideSome = new QAction(tr("Compress Events"), parentWidget);
    actionHideSome->setWhatsThis(tr("Show only a limited number of all day events"));
    connect(actionHideSome, SIGNAL(triggered()), this, SLOT(foldAllDay()));

    QActionGroup *g = new QActionGroup(parentWidget);
    g->setExclusive(true);

    actionToday = new QAction(QIcon(":icon/today"), tr("Today"), g);
    actionToday->setWhatsThis(tr("Show today's events"));
    connect(actionToday, SIGNAL(triggered()), this, SLOT(selectToday()));

    actionDay = new QAction(QIcon(":icon/day"), tr("Day", "day, not date"), g);
    actionDay->setWhatsThis(tr("Show selected day's events"));
    actionDay->setCheckable(true);
    actionDay->setChecked(true);
    connect(actionDay, SIGNAL(triggered()), this, SLOT(viewDay()));

#ifndef QTOPIA_PHONE
    actionWeek = new QAction(QIcon(":icon/week"), tr("Week"), g);
    actionWeek->setWhatsThis(tr("Show selected week's events"));
    actionWeek->setCheckable(true);
    connect(actionWeek, SIGNAL(triggered()), this, SLOT(viewWeek()));

    actionFind = new QAction(QIcon(":icon/find"), tr("Find"), g);
    connect(actionFind, SIGNAL(triggered()), this, SLOT(find()));

    actionNextView = new QAction( QIcon(":icon/month"), tr("Next View"), g);
    actionNextView->setShortcut(QString("*"));
    connect(actionNextView, SIGNAL(triggered()), this, SLOT(nextView()));

    actionBack = new QAction(QIcon(":icon/i18n/back"), tr("Back"), g);
    connect(actionBack, SIGNAL(triggered()), this, SLOT(hideAppointmentDetails()));
#endif

    actionMonth = new QAction(QIcon(":icon/month"), tr("Month"), g);
    actionMonth->setWhatsThis(tr("Show selected month's events"));
    actionMonth->setCheckable(true);
    connect(actionMonth, SIGNAL(triggered()), this, SLOT(viewMonth()));

    actionAccounts = new QAction(QIcon(":icon/settings"), tr("Accounts"), parentWidget);
    connect(actionAccounts, SIGNAL(triggered()), this, SLOT(showAccountSettings()));
    // be default, dont' show this.  dependent on features of model loaded.
    actionAccounts->setVisible(false);

    actionSettings = new QAction(QIcon(":icon/settings"), tr("Settings..."), g);
    connect(actionSettings, SIGNAL(triggered()), this, SLOT(showSettings()));

#if 0
    actionPurge = new QAction( QIcon( ":icon/trash" ), tr( "Purge..." ), g );
    actionPurge->setWhatsThis( tr("Remove old events") );
    connect( actionPurge, SIGNAL(triggered()), this, SLOT( slotPurge() ) );
#endif

    // Setup Menus
#if !defined(QTOPIA_PHONE)
    QToolBar *bar = new QToolBar(this); //(QMainWindow *)parentWidget );
    bar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, bar);

    QMenuBar *mb = menuBar();

#ifndef QTOPIA_NO_POINTER_INPUT
    sub_bar = new QToolBar( this );
    sub_bar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, sub_bar);
#endif

    details_bar = new QToolBar( this );
    details_bar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, details_bar);

    QMenu *appointmentMenu = mb->addMenu( tr( "Event" ) );
    QMenu *view = mb->addMenu( tr( "View" ) );

    appointmentMenu->addAction( actionNew );
    appointmentMenu->addAction( actionEdit );
    appointmentMenu->addAction( actionDelete );
    //appointmentMenu->addAction( actionPurge );

    if (QtopiaSendVia::isDataSupported("text/x-vcalendar"))
        appointmentMenu->addAction( actionBeam );

    appointmentMenu->addSeparator();
    appointmentMenu->addAction( actionFind );

    if ( sub_bar ) {
        sub_bar->addAction( actionNew );
        sub_bar->addAction( actionToday );
        sub_bar->addSeparator();
        QDesktopWidget *desktop = QApplication::desktop();
        bool thinScreen = desktop->availableGeometry(desktop->screenNumber(this)).width() < 200;
        if ( !thinScreen ) {
            sub_bar->addAction( actionDay );
            sub_bar->addAction( actionWeek );
            sub_bar->addAction( actionMonth );
        }
        //sub_bar->addAction( actionFind );
    }

    if ( details_bar ) {
        details_bar->addAction( actionEdit );
        details_bar->addAction( actionDelete );

        if (QtopiaSendVia::isDataSupported("text/x-vcalendar"))
            details_bar->addAction( actionBeam );

        details_bar->addSeparator();
        details_bar->addAction( actionBack );

        details_bar->hide();
    }

    view->addAction( actionToday );
    view->addSeparator();
    view->addAction( actionDay );
    view->addAction( actionWeek );
    view->addAction( actionMonth );
    view->addSeparator();
    view->addAction( actionAccounts );
    view->addAction( actionSettings );

#else
    contextMenu = QSoftMenuBar::menuFor(parentWidget);

    contextMenu->addAction( actionNew );
    contextMenu->addAction( actionEdit );
    contextMenu->addAction( actionDelete );
    contextMenu->addAction( actionAccounts );
    contextMenu->addAction( actionShowAll );
    contextMenu->addAction( actionHideSome );
    //contextMenu->actionPurge( actionPurge );

    if (QtopiaSendVia::isDataSupported("text/x-vcalendar"))
        contextMenu->addAction( actionBeam );

    contextMenu->addAction( actionToday );
    contextMenu->addAction( actionMonth );
    contextMenu->addAction( actionAccounts );
    contextMenu->addAction( actionSettings );
#endif
}

