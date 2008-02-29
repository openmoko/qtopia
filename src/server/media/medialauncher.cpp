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

#include "medialauncher.h"
#include "medialauncherview.h"
#include "launcherview.h"

#include <qcontent.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QStackedWidget>
#include <QCloseEvent>

MediaLauncher::MediaLauncher(QSettings&, QWidget *parent, Qt::WFlags fl)
    : QWidget(parent, fl)
{
    setFont(QFont("helvetica", 24));
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->screenGeometry(desktop->primaryScreen()));

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    stack = new QStackedWidget;
    layout->addWidget(stack);

    launcher = new MediaLauncherView;
    connect(launcher, SIGNAL(clicked(QContent)),
            this, SLOT(execute(QContent)));
    stack->addWidget(launcher);
}

MediaLauncher::~MediaLauncher()
{
}


void MediaLauncher::createGUI()
{
}

void MediaLauncher::showGUI()
{
    show();
    raise();
}

void MediaLauncher::destroyGUI()
{

}

void MediaLauncher::execute(const QContent *content)
{
    if (!content)
        return;
    if (!content->executableName().isNull()) {
        content->execute();
    } else {
        QString type = content->type();
        if (views.contains(type)) {
            showView(type);
        } else if (type.startsWith("Folder/")) {
            LauncherView *v = new ApplicationLauncherView(type.mid(7), stack);
            v->setViewMode(LauncherView::List);
            connect(v, SIGNAL(clicked(const QContent*)),
                    this, SLOT(execute(const QContent*)));
            stack->addWidget(v);
            views[type] = v;
            showView(type);
        }
    }
}

void MediaLauncher::showView(const QString &type)
{
    if (views.contains(type))
        stack->setCurrentWidget(views[type]);
}

void MediaLauncher::closeEvent(QCloseEvent *ce)
{
    ce->ignore();
    if (stack->currentWidget() != launcher)
        stack->setCurrentWidget(launcher);
}

