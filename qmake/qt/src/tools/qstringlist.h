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

#ifndef QSTRINGLIST_H
#define QSTRINGLIST_H

#ifndef QT_H
#include "qvaluelist.h"
#include "qstring.h"
#include "qstrlist.h"
#endif // QT_H

#ifndef QT_NO_STRINGLIST

class QRegExp;
template <class T> class QDeepCopy;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
//Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<QString>; 
// MOC_SKIP_END
#endif

class Q_EXPORT QStringList : public QValueList<QString>
{
public:
    QStringList() { }
    QStringList( const QStringList& l ) : QValueList<QString>(l) { }
    QStringList( const QValueList<QString>& l ) : QValueList<QString>(l) { }
    QStringList( const QString& i ) { append(i); }
#ifndef QT_NO_CAST_ASCII
    QStringList( const char* i ) { append(i); }
#endif

    static QStringList fromStrList(const QStrList&);

    void sort();

    static QStringList split( const QString &sep, const QString &str, bool allowEmptyEntries = FALSE );
    static QStringList split( const QChar &sep, const QString &str, bool allowEmptyEntries = FALSE );
#ifndef QT_NO_REGEXP
    static QStringList split( const QRegExp &sep, const QString &str, bool allowEmptyEntries = FALSE );
#endif
    QString join( const QString &sep ) const;

    QStringList grep( const QString &str, bool cs = TRUE ) const;
#ifndef QT_NO_REGEXP
    QStringList grep( const QRegExp &expr ) const;
#endif

    QStringList& gres( const QString &before, const QString &after,
		       bool cs = TRUE );
#ifndef QT_NO_REGEXP_CAPTURE
    QStringList& gres( const QRegExp &expr, const QString &after );
#endif

protected:
    void detach() { QValueList<QString>::detach(); }
    friend class QDeepCopy< QStringList >;
};

#ifndef QT_NO_DATASTREAM
class QDataStream;
extern Q_EXPORT QDataStream &operator>>( QDataStream &, QStringList& );
extern Q_EXPORT QDataStream &operator<<( QDataStream &, const QStringList& );
#endif

#endif // QT_NO_STRINGLIST
#endif // QSTRINGLIST_H
