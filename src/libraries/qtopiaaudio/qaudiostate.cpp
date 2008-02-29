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

#include <qaudiostate.h>
#include <QAudioStateInfo>

/*!
    \class QAudioState
    \brief The QAudioState class encapsulates an audio device state.

    The QAudioState class can be used to implement a particular
    state for a hardware audio device.  For instance, a state
    where the output of the phone modem is routed to the earphones.

*/

/*!
    Creates a new Audio State object with QObject parent
    set to \a parent.
*/
QAudioState::QAudioState(QObject *parent)
    : QObject(parent)
{

}

/*!
    Destroys an Audio State object.
*/
QAudioState::~QAudioState()
{

}

/*!
    Returns the information about the audio state.

    \sa capabilities(), QAudioStateInfo
*/
QAudioStateInfo QAudioState::info() const
{
    return QAudioStateInfo();
}

/*!
    Returns the capabilities that this audio state supports.

    \sa info()
*/
QAudio::AudioCapabilities QAudioState::capabilities() const
{
    return QAudio::None;
}

/*!
    Returns true whether the state is available.
    E.g. Bluetooth state is only available if the Bluetooth headset
    is plugged in.

    \sa availabilityChanged()
*/
bool QAudioState::isAvailable() const
{
    return false;
}

/*!
    Asks the audio state object to enter its supported audio
    state with \a capability.

    The function returns true if the state transition could be
    performed.

    \sa leave(), capabilities()
*/
bool QAudioState::enter(QAudio::AudioCapability capability)
{
    Q_UNUSED(capability)
    return false;
}

/*!
    Asks the audio state object to leave its supported
    audio state.

    The function returns true if the state transition could
    be performed.

    \sa enter()
*/
bool QAudioState::leave()
{
    return false;
}

/*!
    \fn void QAudioState::availabilityChanged(bool available);

    This signal is emitted by the audio state whenever the availability
    state has changed. The new availability is given by \a available.
    For instance, the headset might have been plugged in and is now
    available for use.

    \sa isAvailable()
*/

/*!
    \fn void QAudioState::doNotUseHint();

    This signal is emitted by the audio state whenever it has received
    a hint that it should no longer be used.  For instance, this could
    be a user-initiated action from the hardware device (e.g. Bluetooth
    multi-function button pressed).

    \sa useHint()
*/

/*!
    \fn void QAudioState::useHint();

    This signa is emitted by the audio state whenever it has received
    a hint that it should be the state being used.

    \sa doNotUseHint()
*/
