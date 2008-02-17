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

#include <qcallvolume.h>

/*!
    \class QCallVolume
    \mainclass
    \preliminary
    \brief The QCallVolume class provides access to GSM call volume control.
    \ingroup telephony

    The current speaker volume may be queried with speakerVolume().  The range
    of supported volume levels may be queried with minimumSpeakerVolume() and
    maximumSpeakerVolume().  The speaker volume can be set with setSpeakerVolume().

    The current microphone volume may be queried with microphoneVolume().  The
    range of supported microphone volume levels may be queried with
    minimumMicrophoneVolume() and maximumMicrophoneVolume().  The microphone
    volume can be set with setMicrophoneVolume().

    \sa QCommInterface
*/

/*!
    \fn void QCallVolume::speakerVolumeChanged(int volume)

    This signal is emitted when the speaker volume changes, to \a volume.
*/

/*!
    \fn void QCallVolume::microphoneVolumeChanged(int volume)

    This signal is emitted when the microphone volume changes to \a volume.
*/

/*!
    Construct a new call volume object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports call volume.  If there is more
    than one service that supports call volume, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QCallVolume objects for each.

    \sa QCommServiceManager::supports()
*/
QCallVolume::QCallVolume
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QCallVolume", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this call volume object.
*/
QCallVolume::~QCallVolume()
{
}

/*!
    \property QCallVolume::speakerVolume
    \brief the current speaker volume.
*/
int QCallVolume::speakerVolume() const
{
    return value( "SpeakerVolume" ).toInt();
}

/*!
    \property QCallVolume::minimumSpeakerVolume
    \brief the minimum speaker volume.
*/
int QCallVolume::minimumSpeakerVolume() const
{
    return value ( "MinimumSpeakerVolume" ).toInt();
}

/*!
    \property QCallVolume::maximumSpeakerVolume
    \brief the maximum speaker volume.
*/
int QCallVolume::maximumSpeakerVolume() const
{
    return value ( "MaximumSpeakerVolume" ).toInt();
}

/*!
    \property QCallVolume::microphoneVolume
    \brief the current microphone volume.
*/
int QCallVolume::microphoneVolume() const
{
    return value( "MicrophoneVolume" ).toInt();
}

/*!
    \property QCallVolume::minimumMicrophoneVolume
    \brief the minimum microphone volume.
*/
int QCallVolume::minimumMicrophoneVolume() const
{
    return value ( "MinimumMicrophoneVolume" ).toInt();
}

/*!
    \property QCallVolume::maximumMicrophoneVolume
    \brief the maximum microphone volume.
*/
int QCallVolume::maximumMicrophoneVolume() const
{
    return value ( "MaximumMicrophoneVolume" ).toInt();
}

void QCallVolume::setSpeakerVolume( int volume )
{
    invoke( SLOT(setSpeakerVolume(int)), volume );
}

void QCallVolume::setMicrophoneVolume( int volume )
{
    invoke( SLOT(setMicrophoneVolume(int)), volume );
}

