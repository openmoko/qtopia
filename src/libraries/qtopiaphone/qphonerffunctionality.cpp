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

#include <qphonerffunctionality.h>

/*!
    \class QPhoneRfFunctionality
    \mainclass
    \brief The QPhoneRfFunctionality class provides a method to get or set the level of RF activity on a phone.
    \ingroup telephony

    The RF activity level is typically used to implement airplane-safe operation
    modes where RF activity is disabled.

    \sa QCommInterface
*/

/*!
    Construct a new phone RF functionality object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports phone RF functionality.  If there is more
    than one service that supports phone RF functionality, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QPhoneRfFunctionality objects for each.

    \sa QCommServiceManager::supports()
*/
QPhoneRfFunctionality::QPhoneRfFunctionality
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QPhoneRfFunctionality", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this phone RF functionality object.
*/
QPhoneRfFunctionality::~QPhoneRfFunctionality()
{
}

/*!
    \enum QPhoneRfFunctionality::Level
    This enum defines the level of phone RF functionality desired.

    \value Minimum Minimum functionality.
    \value Full Full functionality.
    \value DisableTransmit Disable phone transmit RF circuits only.
    \value DisableReceive Disable phone receive RF circuits only.
    \value DisableTransmitAndReceive Disable both phone transmit and receive RF circuits.
*/

/*!
    \property QPhoneRfFunctionality::level
    \brief the current phone RF functionality level.
*/

/*!
    Returns the current phone RF functionality level.

    \sa setLevel(), levelChanged()
*/
QPhoneRfFunctionality::Level QPhoneRfFunctionality::level() const
{
    return qVariantValue<QPhoneRfFunctionality::Level>
        ( value( "level", qVariantFromValue( Minimum ) ) );
}

/*!
    Force the modem to query the current RF functionality level.
    This is typically used just after power up to check the actual
    state of the modem.

    \sa levelChanged()
*/
void QPhoneRfFunctionality::forceLevelRequest()
{
    invoke( SLOT(forceLevelRequest()) );
}

/*!
    Sets the current phone RF functionality \a level.  The result of changing
    the level is reported via the setLevelResult() signal.

    \sa setLevelResult(), level(), levelChanged()
*/
void QPhoneRfFunctionality::setLevel( QPhoneRfFunctionality::Level level )
{
    invoke( SLOT(setLevel(QPhoneRfFunctionality::Level)),
            qVariantFromValue( level ) );
}

/*!
    \fn void QPhoneRfFunctionality::levelChanged()

    Signal that is emitted when the RF functionality level changes.

    \sa level(), setLevel()
*/

/*!
    \fn void QPhoneRfFunctionality::setLevelResult( QTelephony::Result result )

    Signal that is emitted in response to setLevel() to report its \a result.
    If \a result is QTelephony::OK, then level() will also be updated to
    reflect the new phone RF functionality level.

    \sa setLevel()
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QPhoneRfFunctionality::Level)
