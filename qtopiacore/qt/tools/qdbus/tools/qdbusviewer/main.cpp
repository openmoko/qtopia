/****************************************************************************
**
** Copyright (C) 2004-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/qapplication.h>
#include <QtGui/qmainwindow.h>
#include <QtGui/qtabwidget.h>
#include <QtDBus/qdbusconnection.h>
#include "qdbusviewer.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow mw;
#ifndef Q_WS_MAC
    app.setWindowIcon(QIcon(QLatin1String(":/trolltech/qdbusviewer/images/qdbusviewer.png")));
#else
    mw.setWindowTitle(qApp->translate("QtDBusViewer", "Qt D-Bus Viewer"));
#endif


    QTabWidget *mainWidget = new QTabWidget;
    mw.setCentralWidget(mainWidget);
    QDBusViewer *sessionBusViewer = new QDBusViewer(QDBusConnection::sessionBus());
    QDBusViewer *systemBusViewer = new QDBusViewer(QDBusConnection::systemBus());
    mainWidget->addTab(sessionBusViewer, QObject::tr("Session Bus"));
    mainWidget->addTab(systemBusViewer, QObject::tr("System Bus"));

    QMenu *fileMenu = mw.menuBar()->addMenu(QObject::tr("&File"));
    QAction *quitAction = fileMenu->addAction(QObject::tr("&Exit"));
    QObject::connect(quitAction, SIGNAL(triggered()), &mw, SLOT(close()));
    quitAction->setShortcuts(QKeySequence::keyBindings(QKeySequence::Close));

    QMenu *helpMenu = mw.menuBar()->addMenu(QObject::tr("&Help"));
    QAction *aboutAction = helpMenu->addAction(QObject::tr("&About"));
    aboutAction->setMenuRole(QAction::AboutRole);
    QObject::connect(aboutAction, SIGNAL(triggered()), sessionBusViewer, SLOT(about()));

    QAction *aboutQtAction = helpMenu->addAction(QObject::tr("About &Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    QObject::connect(aboutQtAction, SIGNAL(triggered()), &app, SLOT(aboutQt()));

    mw.show();

    return app.exec();
}

