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

#include "shutdownsplash.h"
#include <QPixmap>
#include <QLabel>
#include <QApplication>
#include <QDesktopWidget>

/*!
  \class ShutdownSplashScreen
  \ingroup QtopiaServer::Task
  \brief The ShutdownSplashScreen class displays a static splash screen while the system is restarting.

  The ShutdownSplashScreen class displays a full screen image,
  \c {:image/bigwait} while the system is restarting.  It uses the
  SystemShutdownHandler hook to ensure that it is run during restart.  For best
  results, the \c {ShutdownSplashScreen} task should be given highest priority
  for the SystemShutdownHandler to ensure that it appears as soon as possible.

  Note that the shutdown splash screen only appears on system restart and NOT
  on system shutdown.

  The ShutdownSplashScreen class provides the \c {ShutdownSplashScreen} task.
 */

/*! \internal */
bool ShutdownSplashScreen::systemRestart()
{
    QPixmap pix;
    pix = QPixmap(QLatin1String(":image/bigwait"));

    QLabel *lblWait = new QLabel(0);
    lblWait->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                            Qt::WindowStaysOnTopHint);
    lblWait->setAttribute(Qt::WA_DeleteOnClose);
    lblWait->setPixmap(pix);
    lblWait->setAlignment(Qt::AlignCenter);
    QDesktopWidget *desktop = QApplication::desktop();
    lblWait->setGeometry(desktop->screenGeometry(desktop->primaryScreen()));
    lblWait->show();
    return true;
}

QTOPIA_TASK(ShutdownSplashScreen, ShutdownSplashScreen);
QTOPIA_TASK_PROVIDES(ShutdownSplashScreen, SystemShutdownHandler);
