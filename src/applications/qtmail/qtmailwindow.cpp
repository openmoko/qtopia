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



#include "qtmailwindow.h"
#include "writemail.h"

#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <QDebug>
#include <QDrmContentPlugin>

QTMailWindow *QTMailWindow::self = 0;

QTMailWindow::QTMailWindow(QWidget *parent, Qt::WFlags fl)
    : QMainWindow(parent, fl), parentWidget( this ), noShow(false)
{
    QtopiaApplication::loadTranslations("libqtopiamail");
    init();
}

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
    connect(emailClient, SIGNAL(raiseWidget(QWidget*,QString)),
            this, SLOT(raiseWidget(QWidget*,QString)) );
    views->addWidget(emailClient);

    views->setCurrentWidget(emailClient);

    setCentralWidget(views);
    setWindowTitle( emailClient->windowTitle() );

    QDrmContentPlugin::initialize();
}

QTMailWindow::~QTMailWindow()
{
    if (emailClient)
        emailClient->cleanExit( true );
}

void QTMailWindow::closeEvent(QCloseEvent *e)
{
    if (WriteMail* writeMail = emailClient->mWriteMail) {
        if ((views->currentWidget() == writeMail) && (writeMail->hasContent())) {
            // We need to save whatever is currently being worked on
            writeMail->forcedClosure();
        }
    }

    if (emailClient->isTransmitting()) {
        emailClient->closeAfterTransmissionsFinished();
        hide();
        e->ignore();
    } else {
        e->accept();
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
    QMainWindow::setVisible(visible);
}

void QTMailWindow::setDocument(const QString &_address)
{
    emailClient->setDocument(_address);
}

void QTMailWindow::raiseWidget(QWidget *w, const QString &caption)
{
    if (!isVisible())
        showMaximized();

    views->setCurrentWidget(w);
    if (!caption.isEmpty())
        setWindowTitle( caption );

    raise();
    activateWindow();

    // needed to work with context-help
    setObjectName( w->objectName() );
}

QWidget* QTMailWindow::currentWidget() const
{
    return views->currentWidget();
}

QTMailWindow* QTMailWindow::singleton()
{
    return self;
}

