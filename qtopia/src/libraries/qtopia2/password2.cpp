/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/password.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qstring.h>

//////
// START COPIED CODE from password.cpp
//////
#ifndef Q_OS_WIN32
extern "C" char *crypt(const char *key, const char *salt);
#else
static char *crypt(const char *key, const char *salt) {
    //#### revise  
    Q_UNUSED(salt)
    return (char*)key;
}
#endif

static QString qcrypt(const QString& k, const char *salt)
{
    const QCString c_str = k.utf8();
    int		len = k.length();
    QString	result;
    const char	*ptr = c_str;

    for (; len > 8; len -= 8) {
	result += QString::fromUtf8(crypt(ptr, salt));
	ptr += 8;
    }

    if (len) {
	result += QString::fromUtf8(crypt(ptr, salt));
    }

    return result;
}
//////
// END COPIED CODE from password.cpp
//////

bool Password::authenticate(const QString &text)
{
    Config cfg("Security");
    cfg.setGroup("Passcode");
#ifdef QTOPIA_PHONE
    QString passcode = cfg.readEntry("PhoneCode");
#else
    QString passcode = cfg.readEntry("passcode");
#endif
    return (passcode.isEmpty() || qcrypt(text, "a0") == passcode);
}

