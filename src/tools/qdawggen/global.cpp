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



// simplified version of Global::tempDir(). This avoids having to include more
// source files

#include "global.h"

/*!
  \internal
  Returns the default system path for storing temporary files.
  Note: This does not ensure that the provided directory exists.
*/
namespace Qtopia
{
    QString tempDir()
    {
        QString result;
#ifdef Q_OS_UNIX
        result="/tmp/";
#else
        if (getenv("TEMP"))
            result = getenv("TEMP");
        else
            result = getenv("TMP");

        if (result[(int)result.length() - 1] != QDir::separator())
            result.append(QDir::separator());
#endif

        return result;
    }
}

