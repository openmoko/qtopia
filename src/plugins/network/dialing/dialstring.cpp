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
#include "dialstring.h"

/*
   DialUp::dialString() returns the dialstring for the internal
   modem. The dial string initializes the modem for any kind of
   dialup connection. It is likely that this string needs to be
   modified in order to get a connection running.

   The returned string must provide a QString parameter slot for
   the APN.
 */


QString GPRSDialString()
{
    QString dialstring;
//Generic -> tested with Wavecom FASTRACK and Ericsson T39m
    dialstring = "AT+CGDCONT=1,\"IP\",\"%1\" OK "
           "AT+CGATT=1 OK "
           "ATD*99***1#";
    return dialstring;
}

QString GPRSDisconnectString()
{
    QString result;
    result = "\"\" \\d+++\\d\\c OK\nAT+CGATT=0 OK";

    return result;

}

