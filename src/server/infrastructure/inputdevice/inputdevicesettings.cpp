/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include "inputdevicesettings.h"
#include <QtopiaChannel>
#include <QDataStream>
#include "qtopiaserverapplication.h"
#include "qtopiainputevents.h"
#include <stdlib.h>

/*!
  \class InputDeviceSettings
  \ingroup QtopiaServer::Task
  \brief The InputDeviceSettings class allows keyboard and mouse configuration to be adjusted at runtime.

  The current keyboard and mouse driver can be changed, at runtime by sending
  the following QCop messages to the \c {QPE/System} channel:

  \table
  \header \o Message \o Description
  \row \o \c {setMouseProto(QString)}
       \o \i {setMouseProto(QString proto)}

       Set the new mouse protocol to \i proto.  \i proto has the same form as
       would be used in the \c QWS_MOUSE_PROTO environment variable.
  \row \o \c {setKeyboard(QString)}
       \o \i {setKeyboard(QString keyboard)}

       Set the new keyboard driver to \i keyboard.  \i keyboard has the same
       form as would be used in the \c {QWS_KEYBOARD} environment variable.
  \endtable

  The InputDeviceSettings class provides the \c {InputDeviceSettings} task.
  It is part of the Qtopia server and cannot be used by other Qtopia applications.
 */
/*!
  Create a new InputDeviceSettings instance with the specified \a parent.
 */
InputDeviceSettings::InputDeviceSettings(QObject *parent)
: QObject(parent)
{
    QtopiaChannel *channel = new QtopiaChannel("QPE/System", this);
    connect(channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(systemMsg(QString,QByteArray)) );
}

void InputDeviceSettings::systemMsg(const QString &msg, const QByteArray &data)
{
    QDataStream stream( data );

    if ( msg == "setMouseProto(QString)" ) {
        QString mice;
        stream >> mice;
        ::setenv("QWS_MOUSE_PROTO",(const char *)mice.toLatin1(),1);
        QtopiaInputEvents::openMouse();
    } else if ( msg == "setKeyboard(QString)" ) {
        QString kb;
        stream >> kb;
        ::setenv("QWS_KEYBOARD",(const char *)kb.toLatin1(),1);
        QtopiaInputEvents::openKeyboard();

    }
}

QTOPIA_TASK(InputDeviceSettings, InputDeviceSettings);
