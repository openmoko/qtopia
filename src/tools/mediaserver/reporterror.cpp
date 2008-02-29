/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "reporterror.h"

#include <qtopialog.h>

void report_error( ErrorCode code, const char* file, int line )
{
    switch( code )
    {
    case ERR_HELIX:
        qLog(Media) << file << line << "Helix library error.";
        break;
    case ERR_INTERFACE:
        qLog(Media) << file << line << "Interface error.";
        break;
    case ERR_UNSUPPORTED:
        qLog(Media) << file << line << "Unsupported functionality error.";
        break;
    case ERR_UNEXPECTED:
        qLog(Media) << file << line << "Unexpected error.";
        break;
    case ERR_TEST:
        qLog(Media) << file << line << "Test.";
        break;
    default:
        qLog(Media) << file << line << "Unknown error.";
        break;
    }
}
