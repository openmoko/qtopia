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

#include "dialerservice.h"
#include "dialercontrol.h"
#include "phonelauncher.h"
#include "dialercontrol.h"

/*!
    \service DialerService Dialer
    \brief Provides the Qtopia Dialer service.

    The \i Dialer service enables applications to access the system dialer
    to place outgoing calls.
*/

/*!
    \internal
*/
DialerService::~DialerService()
{
}

/*!
    \fn void DialerServer::dialVoiceMail()

    Dial the user's voice mail service.

    This slot corresponds to the QCop service message
    \c{Dialer::dialVoiceMail()}.
*/

/*!
    \fn void DialerService::dial( const QString& name, const QString& number )

    Dial the specified \a number, tagged with the optional \a name.

    This slot corresponds to the QCop service message
    \c{Dialer::dial(QString,QString)}.
*/

/*!
    \fn void DialerService::dial( const QString& number, const QUniqueId& contact )

    Dial the specified \a contact, using the given \a number.

    This slot corresponds to the QCop service message
    \c{Dialer::dial(QUniqueId,QString)}.
*/

/*!
    \fn void DialerService::showDialer( const QString& digits )

    Displays the dialer, preloaded with \a digits.

    This slot corresponds to the QCop service message
    \c{Dialer::showDialer(QString)}.
*/
