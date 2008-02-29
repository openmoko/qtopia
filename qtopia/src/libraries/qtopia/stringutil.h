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


#ifndef QTPALMTOP_stringutil_h__
#define QTPALMTOP_stringutil_h__

#include <qtopia/qpeglobal.h>
#include <qstring.h>

namespace Qtopia
{

// Simplifies white space within each line but keeps the new line characters
QTOPIA_EXPORT QString simplifyMultiLineSpace( const QString &multiLine );

// Creates a QString which doesn't contain any "dangerous"
// characters (i.e. <, >, &, ")
QTOPIA_EXPORT QString  escapeString( const QString& plain );

// Qtopia-specific replacement for QStyleSheet::convertFromPlainText. It's
// like escapeString() but it also handles newlines and whitespace.
QTOPIA_EXPORT QString  escapeMultiLineString( const QString& plain );

QTOPIA_EXPORT QString dehyphenate(const QString&);

// Takes a UTF-8!! string and removes all the XML thingies (entities?)
// from the string and also calls fromUtf8() on it... so make sure
// to pass a QCString/const char* with UTF-8 data only
QTOPIA_EXPORT QString plainString( const char* escaped, unsigned int length );
QTOPIA_EXPORT QString plainString( const QCString& string );

QTOPIA_EXPORT QString plainString( const QString& string );


// collation functions
QTOPIA_EXPORT int compare( const QString & s1, const QString & s2 );
QTOPIA_EXPORT QString buildSortKey( const QString & s );
QTOPIA_EXPORT QString buildSortKey( const QString & s1, const QString & s2 );
QTOPIA_EXPORT QString buildSortKey( const QString & s1, const QString & s2, 
		      const QString & s3 );

}

#endif
