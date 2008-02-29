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

#ifndef MEDIALAUNCHER_H
#define MEDIALAUNCHER_H

#include "serverinterface.h"
#include "launcherview.h"

#include <QWidget>

class QSettings;
class MediaLauncherView;
class QStackedWidget;
class QContent;

class MediaLauncher : public QWidget, public ServerInterface
{
    Q_OBJECT
public:
    MediaLauncher(QSettings&, QWidget *parent = 0, Qt::WFlags fl = 0);
    ~MediaLauncher();

    virtual void createGUI();
    virtual void showGUI();
    virtual void destroyGUI();

protected:
    void closeEvent(QCloseEvent *ce);

private:
    void showView(const QString &type);

    private slots:
        void execute(const QContent *content);

private:
    QStackedWidget *stack;
    MediaLauncherView *launcher;
    QMap<QString,LauncherView*> views;
};

#endif
