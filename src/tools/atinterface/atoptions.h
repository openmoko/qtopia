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

#ifndef ATOPTIONS_H
#define ATOPTIONS_H

#include <qstring.h>
#include <qstringlist.h>

class QTextCodec;

class AtOptions
{
public:
    AtOptions( const QString& startupOptions = QString() );

    bool echo;              // ATE
    char backspace;         // ATS5
    char terminator;        // ATS3
    char response;          // ATS4

    bool suppressResults;   // ATQ
    bool verboseResults;    // ATV
    int extendedErrors;     // AT+CMEE

    QString charset;        // AT+CSCS
    QTextCodec *codec;

    bool cring;             // +CRING unsolicited notification state
    bool ccwa;              // +CCWA unsolicited notification state
    bool clip;              // +CLIP unsolicited notification state
    bool qcam;              // *QCAM unsolicited notification state
    int cind;               // +CMER indicator reporting mode (0-2)
    bool qsq;               // *QSQ signal quality notification state
    bool qbc;               // *QBC battery charge notification state
    int creg;               // +CREG unsolicited notification state

    bool contextSet;        // +CGDCONT context has been set prior to dial
    QString apn;            // +CGDCONT Access Point Name

    int cbstSpeed;          // +CBST speed value
    int cbstName;           // +CBST name value
    int cbstCe;             // +CBST ce value

    int smsService;         // +CSMS value
    bool messageFormat;     // +CMGF value
    bool csdh;              // +CSDH value

    QString phoneStore;     // +CPBS value
    //QString phoneStorePw;   // +CPBS value

    QString startupOptions;
    QStringList startupOptionsList;

    void factoryDefaults();
    void load();
    void save();
    void setCharset( const QString& value );
    bool hasStartupOption( const QString& option );

    void clearDataOptions();
};

#endif // ATOPTIONS_H
