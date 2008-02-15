/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA All rights reserved.
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

#ifndef PACKAGEMANAGER_UTILS_H
#define PACKAGEMANAGER_UTILS_H

#include <qstring.h>

class SizeUtils
{
    public:
    static bool isSufficientSpace(qlonglong size, QString &neededSpace);
    static QString getSizeString( qlonglong size );
    static qlonglong parseInstalledSize( QString installedSize );
};

class LidsUtils
{
    public:
        static bool isLidsEnabled();
};

#endif
