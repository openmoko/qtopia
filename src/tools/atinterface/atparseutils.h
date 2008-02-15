/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef ATPARSEUTILS_H
#define ATPARSEUTILS_H

#include <QString>

class AtParseUtils
{
private:
    AtParseUtils() {}

public:

    /*!
        \enum AtParseUtils::Mode

        This enumeration defines the possible types of AT commands.
        \value CommandOnly  No other characters
        \value Get  Command followed by ? character
        \value Set  Command followed by = character
        \value Support  Command followed by =? characters
        \value Error  Invalid or malformed command

        \sa mode()
    */
    enum Mode
    {
        CommandOnly,            // AT+CFOO only
        Get,                    // AT+CFOO?
        Set,                    // AT+CFOO=value
        Support,                // AT+CFOO=?
        Error                   // Syntax error
    };
    static Mode mode( const QString& params );

};

#endif // ATPARSEUTILS_H
