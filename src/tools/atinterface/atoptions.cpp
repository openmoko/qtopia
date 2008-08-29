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

#include "atoptions.h"
#include <qatutils.h>

AtOptions::AtOptions( const QString& startupOptions )
{
    factoryDefaults();
    load();
    this->startupOptions = startupOptions;
    if ( !startupOptions.isEmpty() )
        startupOptionsList = startupOptions.split(",");
    if ( hasStartupOption( "noecho" ) )
        echo = false;
}

void AtOptions::factoryDefaults()
{
    echo = true;
    backspace = 0x08;
    terminator = 0x0D;
    response = 0x0A;

    suppressResults = false;
    verboseResults = true;
    extendedErrors = 0;

    setCharset( "GSM" );

    cring = false;
    ccwa = false;
    clip = false;
    qcam = false;
    cind = 0;
    cmer = 1;
    qsq = false;
    qbc = false;
    creg = 0;
    cr = false;
    colp = false;
    clae = false;
    cssi = false;
    cssu = false;
    ccwv = false;
    cccm = false;
    lastTimeCCCM = QDateTime::currentDateTime();

    cusd = false;
    cmod = 0;
    csns = 0;

    contextSet = false;
    apn = "internet";

    csgt = false;
    greetingText = "";

    phoneStore = "EN";
    //phoneStorePw = "";

    cbstSpeed = -1;
    cbstName = -1;
    cbstCe = -1;

    cpls = 0;
    cpolFormat = 2;
    //cpolLastIndex = 0;
    dateFormat = 1;
    auxDateFormat = 1;
    timeFormat = 1;
    mtDateTimeOffset = 0;
    mtTimeZoneSeconds = 0;

    csvm = false;
    ignore_ath = false;
    ignore_drop_dtr = true;

    smsService = 0;
    messageFormat = false;
    csdh = false;
}

void AtOptions::load()
{
    // Not used at present.  All sessions start in a reasonable
    // factory default state with no saving from the previous session.
    // Change this if it ever makes sense to save the values.
}

void AtOptions::save()
{
    // Not used at present.
}

void AtOptions::setCharset( const QString& value )
{
    charset = value;
    codec = QAtUtils::codec( value );
}

bool AtOptions::hasStartupOption( const QString& option )
{
    return startupOptions.contains( option );
}

void AtOptions::clearDataOptions()
{
    // Clear data setup options after a dial so that the next
    // dial can start fresh with new settings.
    contextSet = false;
    cbstSpeed = -1;
    cbstName = -1;
    cbstCe = -1;
}

 
// Parse the next string from a parameter list and convert according to codecs.
QString AtOptions::nextString( const QString& buf, uint& posn )
{
    return QAtUtils::decode( QAtUtils::nextString( buf, posn ), codec );
}
