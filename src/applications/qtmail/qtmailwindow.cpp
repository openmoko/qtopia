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



#include "qtmailwindow.h"
#include "writemail.h"

#ifdef QTOPIA_DESKTOP
#include <qcopenvelope_qd.h>
#else
#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>
#endif
#include <qdatetime.h>
#include <qtimer.h>
#include <QDebug>


QTMailWindow *QTMailWindow::self = 0;

#ifdef QTOPIA_DESKTOP
QTMailWindow::QTMailWindow()
    : QTMailGui()
{
}
#else
QTMailWindow::QTMailWindow(QWidget *parent, Qt::WFlags fl)
    : QTMailGui(parent, fl), parentWidget( this ), noShow(false)
{
    init();
}
#endif

void QTMailWindow::init()
{
    self = this;
    views = new QStackedWidget(parentWidget);

#if 0
    // Passing the correct parent for EmailClient does NOT work
    // doing so causes a blank screen to be shown
    emailClient = new EmailClient(views, "client"); // No tr
#else
    // Pass in an incorrect parent, a warning
    // "QLayout::addChildWidget: EmailClient "client" in wrong parent; "
    // "moved to correct parent" will be shown, but this is necessary
    // to make the emailClient QMainWindow display.
    // This seems to be a QMainWindow in a QStackedWidget bug
    emailClient = new EmailClient(this, "client"); // No tr
#endif
    connect(emailClient, SIGNAL( raiseWidget(QWidget*,const QString&) ),
            this, SLOT( raiseWidget(QWidget*,const QString&) ) );
    views->addWidget(emailClient);

    views->setCurrentWidget(emailClient);

#ifndef QTOPIA_DESKTOP
    setCentralWidget(views);
    setWindowTitle( emailClient->windowTitle() );
#endif
}

QTMailWindow::~QTMailWindow()
{
    if (emailClient)
        emailClient->cleanExit( true );
}

void QTMailWindow::closeEvent(QCloseEvent *e)
{
    if (views->currentWidget() != emailClient) {
        //because closeEvent is passed to this instead of sub qtmainwindows

        // We have to test for w = 0 (means none visible) since mWriteMail/mReadMail would
        // also be 0 if they haven't yet been created
        QWidget *w = views->currentWidget();
        if ( w ) {
            if (w == emailClient->mWriteMail ) {
#ifdef QTOPIA_PHONE
                if ( !emailClient->mWriteMail->hasContent() ) {
                    emailClient->writeMailWidget()->discard();
                    e->ignore();
                    return;
                }
                if (!Qtopia::mousePreferred() &&
                    emailClient->mWriteMail->keyPressAccepted()) {
                    e->ignore();
                    return;
                }
                emailClient->writeMailWidget()->tryAccept();
#else
                if( !emailClient->writeMailWidget()->tryAccept() )
                    emailClient->writeMailWidget()->discard();
                return; //call from writemail will be caught by emailclient
#endif
            } else if (w == emailClient->mReadMail ) {
                emailClient->readMailWidget()->close();
                e->ignore();
                return;
            }

            w->hide();
        }

        views->setCurrentWidget(emailClient);
        if (emailClient->focusWidget())
            emailClient->focusWidget()->setFocus();
#ifndef QTOPIA_DESKTOP
        setWindowTitle( emailClient->windowTitle() );
#endif
        emailClient->update();
        // needed to work with context-help
        setObjectName( w->objectName() );

        e->ignore();
    } else {
        if (Qtopia::mousePreferred() &&
            emailClient->currentMailboxWidgetId() == emailClient->currentMessageId() ) {
            emailClient->showFolderList();
            return;
        }

        if (emailClient->isTransmitting()) {
            emailClient->closeAfterTransmissionsFinished();
#ifndef QTOPIA_DESKTOP
            hide();
#endif
            e->ignore();
            return;
        } else {
            e->accept();
        }
    }
}

void QTMailWindow::forceHidden(bool hidden)
{
    noShow = hidden;
}

void QTMailWindow::setVisible(bool visible)
{
    if (noShow && visible)
        return;
    QTMailGui::setVisible(visible);
}

void QTMailWindow::setDocument(const QString &_address)
{
    emailClient->setDocument(_address);
}

void QTMailWindow::raiseWidget(QWidget *w, const QString &caption)
{
    showMaximized();
    if ( caption == "qcop") {
        //the emailclient can't raise itself, only we can
#ifndef QTOPIA_DESKTOP
        raise();
#endif
    } else {
        views->setCurrentWidget(w);
        if (w->focusWidget())
            w->focusWidget()->setFocus(); // Needed, but don't know why.
#ifndef QTOPIA_DESKTOP
        setWindowTitle( caption );
#endif

        // needed to work with context-help
        setObjectName( w->objectName() );
    }
}

QTMailWindow* QTMailWindow::singleton()
{
    return self;
}

