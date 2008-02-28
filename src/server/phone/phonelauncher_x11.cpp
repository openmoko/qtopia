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
/****************************************************************************
**
** Alternative Launcher for X11 integrating with existing environments
**
** Copyright (C) 2008 by OpenMoko, Inc.
** Written by Holger Freyther
** All Rights Reserved
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
****************************************************************************/

#include "phonelauncher_x11.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QX11Info>

#include "qsupplementaryservices.h"

#include "callhistory.h"
#include "contextlabel.h"
#include "qabstractcallpolicymanager.h"
#include "qabstracthomescreen.h"
#include "qabstractmessagebox.h"
#include "themecontrol.h"
#include "windowmanagement.h"

#include "X11/Xlib.h"
#include "X11/Xatom.h"

PhoneLauncherX11::PhoneLauncherX11(QWidget* parent, Qt::WFlags fl)
    : QAbstractServerInterface(parent, fl)
    , m_context(0)
    , m_cbsMessageBox(0)
    , m_serviceMessageBox(0)
    , m_homeScreen(0)
{
    // We are forced to be a QWidget but we do not want to be seen
    setGeometry(0,0,0,0);
    hide();

#ifdef QTOPIA_CELL
    CellBroadcastControl *cbc = CellBroadcastControl::instance();
    connect(cbc,
            SIGNAL(broadcast(CellBroadcastControl::Type,QString,QString)),
            this,
            SLOT(cellBroadcast(CellBroadcastControl::Type,QString,QString)));

    QSupplementaryServices *supp = new QSupplementaryServices( "modem", this );
    connect( supp, SIGNAL(unstructuredNotification(QSupplementaryServices::UnstructuredAction,QString)),
             this, SLOT(unstructuredNotification(QSupplementaryServices::UnstructuredAction,QString)) );
#endif

    // Create phone context bar/soft menu
    createContext();
    m_context->show();

    // Create home screen, important for the call history
    m_homeScreen = qtopiaWidget<QAbstractHomeScreen>(this);
    m_homeScreen->hide();
    HomeScreenControl::instance()->setHomeScreen(m_homeScreen);

    QObject::connect(m_homeScreen, SIGNAL(showCallHistory()),
                     this, SLOT(showCallHistory()));
}

PhoneLauncherX11::~PhoneLauncherX11()
{
    delete m_context;
    delete m_cbsMessageBox;
    delete m_serviceMessageBox;
    delete m_callHistory;
}

/*!
  \internal
  */
void PhoneLauncherX11::createContext()
{
    Q_ASSERT(!m_context);

    m_context = new ContextLabel(0, Qt::FramelessWindowHint |
                                    Qt::Tool |
                                    Qt::WindowStaysOnTopHint );

    m_context->move(QApplication::desktop()->screenGeometry().topLeft()+QPoint(0,50)); // move to the correct screen
    WindowManagement::protectWindow(m_context);
    m_context->setAttribute(Qt::WA_GroupLeader);
    ThemeControl::instance()->registerThemedView(m_context, "Context");
    // Dock now to avoid relayout later.
    WindowManagement::dockWindow(m_context, WindowManagement::Bottom, m_context->reservedSize());

#ifdef Q_WS_X11
    Display* dpy = QX11Info::display();
    Window wId = m_context->winId();
    Q_ASSERT(dpy && wId);

    Atom atom = XInternAtom(dpy, "_QTOPIA_SOFT_MENU", False);
    unsigned long flag = 1;
    XChangeProperty(dpy, wId, atom, XA_CARDINAL, 32, PropModeReplace,
                    (unsigned char*)&flag, 1);
#endif
}

#ifdef QTOPIA_CELL

/*!
  \internal
  */
void PhoneLauncherX11::cellBroadcast(CellBroadcastControl::Type type,
                                  const QString &chan, const QString &text)
{
    if(type != CellBroadcastControl::Popup)
        return;

    // remove the previous message box if there is any
    // user will see the latest one only
    delete m_cbsMessageBox;
    m_cbsMessageBox = QAbstractMessageBox::messageBox(0, chan,
                                   text, QAbstractMessageBox::Information,
                                   QAbstractMessageBox::No);
    QtopiaApplication::showDialog(m_cbsMessageBox);
}

/*!
  \internal
  */
void PhoneLauncherX11::unstructuredNotification
    ( QSupplementaryServices::UnstructuredAction action, const QString& data )
{
    QString text;
    if ( !data.isEmpty() ) {
        text = data;
    } else {
        switch ( action ) {
            case QSupplementaryServices::TerminatedByNetwork:
                text = tr("Operation terminated by network");
                break;

            case QSupplementaryServices::OtherLocalClientResponded:
                text = tr("Other local client has responded");
                break;

            case QSupplementaryServices::OperationNotSupported:
                text = tr("Operation is not supported");
                break;

            case QSupplementaryServices::NetworkTimeout:
                text = tr("Operation timed out");
                break;

            default:
                text = tr("Network response: %1").arg((int)action);
                break;
        }
    }
    ussdMessage(text);
}

#endif // QTOPIA_CELL

void PhoneLauncherX11::ussdMessage(const QString &text)
{
    QString title = tr("Service request");
    QString displayText = "<qt>" + text + "</qt>";
    if (!m_serviceMessageBox) {
        m_serviceMessageBox = QAbstractMessageBox::messageBox(0, title, displayText,
                                       QAbstractMessageBox::Information);
    } else {
        m_serviceMessageBox->setWindowTitle(title);
        m_serviceMessageBox->setText(displayText);
    }
    QtopiaApplication::showDialog(m_serviceMessageBox);
}

/*!
  \internal
  Displays the call history window.  If \a missed is true, the missed
  calls tab will be on top.  If \a hint is not empty, it is used to
  highlight a matching call.
*/
void PhoneLauncherX11::showCallHistory(bool missed, const QString &hint)
{
#if defined(QTOPIA_TELEPHONY)
    if ( !callHistory() )
        initializeCallHistory();

    callHistory()->reset();
    if (missed || DialerControl::instance()->missedCallCount() > 0)
        callHistory()->showMissedCalls();
    if( hint.length() )
        callHistory()->setFilter( hint );

    callHistory()->refresh();

    if( !callHistory()->isHidden() )
    {
        callHistory()->raise();
    }
    else
    {
        callHistory()->showMaximized();
    }
#endif
}

/*!
  \internal
  Initialize call history window.
  This delayed initilization will shorten the first launch time.
*/
void PhoneLauncherX11::initializeCallHistory()
{
#if defined(QTOPIA_TELEPHONY)
    if ( !m_callHistory ) {
        m_callHistory = new CallHistory(DialerControl::instance()->callList(), 0);
        QtopiaApplication::prepareMainWidget(m_callHistory);
        connect(callHistory(), SIGNAL(viewedMissedCalls()),
                this, SLOT(resetMissedCalls()) );
        connect(callHistory(), SIGNAL(viewedMissedCalls()),
                DialerControl::instance(), SLOT(resetMissedCalls()) );
        connect(callHistory(),
                SIGNAL(requestedDial(QString,QUniqueId)),
                this,
                SLOT(requestDial(QString,QUniqueId)));
    }
#endif
}
#endif

QTOPIA_REPLACE_WIDGET_OVERRIDE(QAbstractServerInterface, PhoneLauncherX11);

#include "phonelauncher_x11.moc"
