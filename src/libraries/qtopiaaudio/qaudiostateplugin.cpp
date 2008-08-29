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

#include "qaudiostateplugin.h"

#include <QSet>
#include <QPair>

/*!
  \class QAudioStatePlugin

  \brief The QAudioStatePlugin class provides a way of managing the underlying audio system.

  This class needs to be implemented as part of the device profile (devices/devicename/src/plugins/audiohardware/devicename)

  In the implementation multiple QAudioState's are created to handle each different
  audio state required on the device.

  The list of QAudioState's available are returned to the manager by QAudioStatePlugin::statesProvided().
  Each of these QAudioState implementations are interogated using QAudioState::isAvailable()
  QAudioState::capabilities() and QAudioState::info() by the manager to determine what should be the current state.

  When a state is no longer available or has become available QAudioState::availabilityChanged( bool available)
  should be emitted by the QAudioState implementation to let the manager know it needs to find a new state and change into it.

  Example QAudioState implementations:

  - Headset (domain="Phone", priority=50)

  When a headset is plugged into the device and you are on a call (domain=Phone), the QAudioState implementation would emit the
  QAudioState::availabilityChanged(true) since it has the high priority (lowest value is highest priority),
  the manager will switch into it. When the headset is unplugged during the call (domain=Phone), the QAudioState implementation
  would emit the QAudioState::availabilityChanged(false), the manager would then switch to the next best available QAudioState.

  - MediaSpeaker (domain="Media", priority=150)

  This is the default, lowest priority.

  - Headphones (domain="Media", priority=25)

  When you are listening to music on loud speaker and you plugin the headphones, emits available and manager switches into it.

  - Speakerphone (domain="Phone", priority=100)

  You are on a call and you press the speaker phone button, the key is handled by setting a QValueSpace object that the
  QAudioState implementation is listening on, available is emitted and manager switches into it.

  Depending on what is available at any moment the highest priority is chosen to suit the required domain and capabilites
  available.

  QAudioState::capabilities() returns QAudio, this can be set to be input, output or input and output.

  */

/*!
    Constructs a QAudioStatePlugin object with parent set to \a parent.
*/
QAudioStatePlugin::QAudioStatePlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys a QAudioStatePlugin object.
*/

QAudioStatePlugin::~QAudioStatePlugin()
{
}

/*!
    Returns a list of all QAudioStates provided by this plugin.
*/
QList<QAudioState *> QAudioStatePlugin::statesProvided() const
{
    return QList<QAudioState *>();
}
