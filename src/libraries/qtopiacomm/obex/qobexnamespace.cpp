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

#include <qobexnamespace.h>

/*!
    \enum QObex::State
    Defines the States that an OBEX client or service can be in.

    \value Ready The client or service is ready to send or receive requests. This is the default state.
    \value Connecting A connection request is in progress.
    \value Disconnecting A disconnection request is in progress.
    \value Streaming A get or put request is in progress.
    \value Closed The client or service has been closed and cannot process any more requests.
 */

/*!
    \enum QObex::Error
    Defines the Error states that an OBEX client or service can be in.

    \value NoError No error has occurred.
    \value LinkError A connection link has been interrupted.
    \value TransportConnectionError Error while connecting the underlying socket transport.
    \value RequestFailed The client's request was refused by the remote service, or an error occured while sending the request.
    \value UnknownError An error other than those specified above occurred.
 */

