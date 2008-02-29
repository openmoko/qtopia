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

#include <qtopiaglobal.h>

#include <stdlib.h>

#include <QTimer>
#include <QPainter>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStyle>

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qmimetype.h>

#include "runningappbar.h"

#include <server.h>
#include <contentserver.h>

RunningAppBar::RunningAppBar(QWidget* parent)
  : QFrame(parent), selectedAppIndex(-1)
{
    QtopiaChannel* channel = new QtopiaChannel( "QPE/System", this );
    connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(received(const QString&,const QByteArray&)) );

    spacing = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
}

RunningAppBar::~RunningAppBar()
{
}

void RunningAppBar::received(const QString& msg, const QByteArray& data) {
    // Since fast apps appear and disappear without disconnecting from their
    // channel we need to watch for the showing/hiding events and update according.
#ifdef QTOPIA4_TODO
    QDataStream stream( data );
    if ( msg == "fastAppShowing(QString)") {
        QString appName;
        stream >> appName;
        const QContent f = Server::instance()->appLoader()->findExec(appName);
        if ( f.isValid() )
            addTask(f);
    } else if ( msg == "fastAppHiding(QString)") {
        QString appName;
        stream >> appName;
        const QContent f = Server::instance()->appLoader()->findExec(appName);
        if ( f.isValid() )
            removeTask(f);
    }
#endif
}

void RunningAppBar::addTask(const QContent& appLnk)
{
    QContent* newApp = new QContent(appLnk);
    newApp->setExecutableName(appLnk.executableName());
    appList.prepend(newApp);
    update();
}

void RunningAppBar::removeTask(const QContent& appLnk)
{
    QMutableListIterator<QContent*> it(appList);
    while (it.hasNext()) {
        QContent* target = it.next();
        if (target->executableName() == appLnk.executableName()) {
            it.remove();
            delete target;
        }
    }
    update();
}

void RunningAppBar::mousePressEvent(QMouseEvent *e)
{
    // Find out if the user is clicking on an app icon...
    // If so, snag the index so when we repaint we show it
    // as highlighed.
    selectedAppIndex = 0;
    int x = 0;
    QListIterator<QContent*> it(appList);
    while (it.hasNext()) {
        it.next();
        if ( x + spacing <= width() ) {
            if ( e->x() >= x && e->x() < x+spacing ) {
                if ( selectedAppIndex < (int)appList.count() ) {
                    repaint(rect());
                    return;
                }
            }
        } else {
            break;
        }
        ++selectedAppIndex;
        x += spacing;
    }
    selectedAppIndex = -1;
    repaint(rect());
}

void RunningAppBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
        return;
    if ( selectedAppIndex >= 0 ) {
        QString app = appList.at(selectedAppIndex)->executableName();
        QtopiaIpcEnvelope e("QPE/System", "raise(QString)");
        e << app;
        selectedAppIndex = -1;
        update();
    }
}

void RunningAppBar::paintEvent( QPaintEvent * )
{
    const int sz = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
    QPainter p( this );
    int x = 0;
    int y = (height() - sz) / 2;

    QContent *curApp;
    foreach (QContent *app, appList) {
        if ( x + spacing <= width() ) {
            curApp = app;
            p.drawPixmap( x, y, curApp->icon().pixmap(sz) );
            x += spacing;
        }
    }
}

QSize RunningAppBar::sizeHint() const
{
    return QSize( frameWidth(), qApp->style()->pixelMetric(QStyle::PM_SmallIconSize)+frameWidth()*2+3 );
}

void RunningAppBar::applicationLaunched(const QString &appName)
{
    QContentSet set(QContent::Application);
    QContent newGuy = set.findExecutable(appName);
    if ( newGuy.isValid() && !newGuy.isPreloaded() ) {
        addTask( newGuy );
    }
}

void RunningAppBar::applicationTerminated(const QString &app)
{
    QContentSet set(QContent::Application);
    QContent gone = set.findExecutable(app);
    if ( gone.isValid() ) {
        removeTask(gone);
    }
}

