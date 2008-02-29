/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qmodemvibrateaccessory.h>
#include <qmodemservice.h>

/*!
    \class QModemVibrateAccessory
    \mainclass
    \brief The QModemVibrateAccessory class provides vibrate accessory support for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CVIB} command from 3GPP TS 27.007.

    QModemVibrateAccessory implements the QVibrateAccessory hardware interface.  Client
    applications should use QVibrateAccessory instead of this class to
    access the modem's vibration settings.

    \sa QVibrateAccessory
*/

/*!
    Create an AT-based vibrate accessory for \a service.
*/
QModemVibrateAccessory::QModemVibrateAccessory( QModemService *service )
    : QVibrateAccessoryProvider( service->service(), service )
{
    this->service = service;
    setSupportsVibrateOnRing( true );
}

/*!
    Destroy this AT-based vibrate accessory.
*/
QModemVibrateAccessory::~QModemVibrateAccessory()
{
}

/*!
    \reimp
*/
void QModemVibrateAccessory::setVibrateOnRing( const bool value )
{
    if ( value )
        service->chat( "AT+CVIB=1" );
    else
        service->chat( "AT+CVIB=0" );
    QVibrateAccessoryProvider::setVibrateOnRing( value );
}

/*!
    Sets the vibrateNow() state to \a value.  This is not used in this
    implementation because \c{AT+CVIB} does not support immediate vibrate.
    Subclasses should override this if the modem supports immediate vibrate.
*/
void QModemVibrateAccessory::setVibrateNow( const bool value )
{
    // Not used.
    Q_UNUSED(value);
}
