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

#include "shutdownsplash.h"
#include <QWaitWidget>

/*!
  \class ShutdownSplashScreen
  \ingroup QtopiaServer::Task
  \brief The ShutdownSplashScreen class displays a static splash screen while the system is restarting.

  The ShutdownSplashScreen provides a Qtopia Server Task.  Qtopia Server Tasks 
  are documented in full in the QtopiaServerApplication class documentation.

  \table
  \row \o Task Name \o ShutdownSplashScreen
  \row \o Interfaces \o SystemShutdownHandler
  \row \o Services \o None
  \endtable

  The ShutdownSplashScreen class displays a full screen image,
  while the system is restarting.  It uses the
  SystemShutdownHandler hook to ensure that it is run during restart.  For best
  results, the \c {ShutdownSplashScreen} task should be given highest priority
  for the SystemShutdownHandler to ensure that it appears as soon as possible.

  \i {Note:} The shutdown splash screen only appears on system restart and NOT
  on system shutdown.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
 */

/*! \internal */
bool ShutdownSplashScreen::systemRestart()
{
    QWaitWidget* w = new QWaitWidget( 0 );
    w->setText(tr("Please wait..."));
    w->setWindowFlags(Qt::WindowStaysOnTopHint);
    w->setWindowState(Qt::WindowFullScreen);
    w->setAttribute( Qt::WA_DeleteOnClose );
    w->setCancelEnabled( false );
    w->showFullScreen();
    return true;
}

QTOPIA_TASK(ShutdownSplashScreen, ShutdownSplashScreen);
QTOPIA_TASK_PROVIDES(ShutdownSplashScreen, SystemShutdownHandler);
