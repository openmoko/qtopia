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
#ifndef TYPECONVERSION_H
#define TYPECONVERSION_H

#include <qdwin32config.h>
#include <windows.h>
#include <tchar.h>
#include <QString>

namespace QDWIN32 {

QDWIN32_EXPORT QString tchar_to_qstring( TCHAR *string, int length );
QDWIN32_EXPORT TCHAR *qstring_to_tchar( const QString &string );

};

#endif
