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

#include "atparseutils.h"

/*!
    \class AtParseUtils
    \brief The AtParseUtils class provides several utility functions that assist with parsing AT commands
    \mainclass
*/

/*!
    \fn AtParseUtils::Mode AtParseUtils::mode( const QString& params )
    Determines the mode that an extension command is operating in
    according to the \a params to the command.
    Returns the mode of the command.
*/
AtParseUtils::Mode AtParseUtils::mode( const QString& params )
{
    if ( params.isEmpty() )
        return CommandOnly;
    if ( params[0] == QChar('=') ) {
        if ( params.length() > 1 && params[1] == QChar('?') ) {
            if ( params.length() != 2 )
                return Error;
            else
                return Support;
        } else {
            return Set;
        }
    }
    if ( params[0] == QChar('?') ) {
        if ( params.length() != 1 )
            return Error;
        else
            return Get;
    }
    return Error;
}

