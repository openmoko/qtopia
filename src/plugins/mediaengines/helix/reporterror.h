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

#ifndef REPORTERROR_H
#define REPORTERROR_H

#define REPORT_ERROR(code) report_error(code,__FILE__,__LINE__)

enum ErrorCode {
    ERR_UNKNOWN,
    ERR_HELIX,
    ERR_INTERFACE,
    ERR_UNSUPPORTED,
    ERR_UNEXPECTED,
    ERR_TEST
};

void report_error( ErrorCode code, const char * file, int line );

#endif // REPORTERROR_H
