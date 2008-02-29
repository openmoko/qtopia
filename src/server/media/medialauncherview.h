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

#ifndef MEDIALAUNCHERVIEW_H
#define MEDIALAUNCHERVIEW_H

#include <QWidget>

class QContent;
class QListWidget;
class QListWidgetItem;

class MediaLauncherViewPrivate;

class MediaLauncherView : public QWidget
{
    Q_OBJECT
public:
    MediaLauncherView(QWidget *parent=0, Qt::WindowFlags flags=0);

signals:
    void clicked(const QContent *);

private:
    void load();
    QContent readEntry(const QString &entry);

private slots:
    void activated(QListWidgetItem *item);

private:
    QListWidget *view;
    MediaLauncherViewPrivate *d;
};

#endif
