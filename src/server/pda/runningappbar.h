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

#ifndef RUNNING_APP_BAR_H
#define RUNNING_APP_BAR_H

#include <qcontent.h>


#include <qframe.h>
#include <qlist.h>
#include <qtimer.h>
#include <qmap.h>
#include <qpointer.h>

class QContentSet;
class QProcess;
class QMessageBox;
class TempScreenSaverMonitor;

class RunningAppBar : public QFrame
{
    Q_OBJECT

public:
    RunningAppBar(QWidget* parent);
    ~RunningAppBar();

    QSize sizeHint() const;

protected:
    void addTask(const QContent& appLnk);
    void removeTask(const QContent& appLnk);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

public slots:
    void applicationLaunched(const QString &);
    void applicationTerminated(const QString &);

private slots:
    void received(const QString& msg, const QByteArray& data);

private:
    const QContentSet *appLnkSet;
    QList<QContent*> appList;
    int selectedAppIndex;
    int spacing;
};


#endif

