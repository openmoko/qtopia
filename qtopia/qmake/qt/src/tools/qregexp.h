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

#ifndef QREGEXP_H
#define QREGEXP_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_REGEXP
class QRegExpEngine;
struct QRegExpPrivate;

class Q_EXPORT QRegExp
{
public:
    enum CaretMode { CaretAtZero, CaretAtOffset, CaretWontMatch };

    QRegExp();
    QRegExp( const QString& pattern, bool caseSensitive = TRUE,
	     bool wildcard = FALSE );
    QRegExp( const QRegExp& rx );
    ~QRegExp();
    QRegExp& operator=( const QRegExp& rx );

    bool operator==( const QRegExp& rx ) const;
    bool operator!=( const QRegExp& rx ) const { return !operator==( rx ); }

    bool isEmpty() const;
    bool isValid() const;
    QString pattern() const;
    void setPattern( const QString& pattern );
    bool caseSensitive() const;
    void setCaseSensitive( bool sensitive );
#ifndef QT_NO_REGEXP_WILDCARD
    bool wildcard() const;
    void setWildcard( bool wildcard );
#endif
    bool minimal() const;
    void setMinimal( bool minimal );

    bool exactMatch( const QString& str ) const;
#ifndef QT_NO_COMPAT
    int match( const QString& str, int index = 0, int *len = 0,
	       bool indexIsStart = TRUE ) const;
#endif

#if defined(Q_QDOC)
    int search( const QString& str, int offset = 0,
		CaretMode caretMode = CaretAtZero ) const;
    int searchRev( const QString& str, int offset = -1,
		   CaretMode caretMode = CaretAtZero ) const;
#else
    // ### Qt 4.0: reduce these four to two functions
    int search( const QString& str, int offset = 0 ) const;
    int search( const QString& str, int offset, CaretMode caretMode ) const;
    int searchRev( const QString& str, int offset = -1 ) const;
    int searchRev( const QString& str, int offset, CaretMode caretMode ) const;
#endif
    int matchedLength() const;
#ifndef QT_NO_REGEXP_CAPTURE
    int numCaptures() const;
    QStringList capturedTexts();
    QString cap( int nth = 0 );
    int pos( int nth = 0 );
    QString errorString();
#endif

    static QString escape( const QString& str );

private:
    void prepareEngine() const;
    void prepareEngineForMatch( const QString& str ) const;
    void invalidateEngine();

    static int caretIndex( int offset, CaretMode caretMode );

    QRegExpEngine *eng;
    QRegExpPrivate *priv;
};
#endif // QT_NO_REGEXP
#endif // QREGEXP_H
