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

#include "virtualkeyboardservice.h"
#include <qtopiaipcenvelope.h>
#include <qwindowsystem_qws.h>
#include "qtopiaserverapplication.h"
#include <QCopChannel>

/*!
  \class VirtualKeyboardService
  \ingroup QtopiaServer::Task
  \brief The VirtualKeyboardService class provides a service to simulate keypresses on a device.

  The \c {VirtualKeyboard} service provided by the VirtualKeyboardService class
  allows applications to cause the equivalent of physical keypresses on a
  device.

  The VirtualKeyboardService class provides the \c {VirtualKeyboard} task.
 */

/*!
  Construct a new VirtualKeyboardService instance with the provided \a parent.
 */
VirtualKeyboardService::VirtualKeyboardService( QObject *parent )
    : QtopiaAbstractService( "VirtualKeyboard", parent )
{
    publishAll();
}

/*!
  Destroys the VirtualKeyboardService instance.
 */
VirtualKeyboardService::~VirtualKeyboardService()
{
}

/*!
  Simulate the press and release of \a key.  This is equivalent to calling the
  QWSServer::processKeyEvent() method for a press and then a release.

  This slot corresponds to the QCop service message \c{VirtualKeyboard::keyPress(int)}.
 */
void VirtualKeyboardService::keyPress( int key )
{
    qwsServer->processKeyEvent(QChar(key).unicode(), key, 0, true, false);
    qwsServer->processKeyEvent(QChar(key).unicode(), key, 0, false, false);
}

QTOPIA_TASK(VirtualKeyboard, VirtualKeyboardService);
