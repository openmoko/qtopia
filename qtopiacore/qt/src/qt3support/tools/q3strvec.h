/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3STRVEC_H
#define Q3STRVEC_H

#include <QtCore/qstring.h>
#include <Qt3Support/q3ptrvector.h>
#include <QtCore/qdatastream.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3StrVec : public Q3PtrVector<char>
{
public:
    Q3StrVec()  { dc = true; }
    Q3StrVec( uint size, bool deepc = true ) : Q3PtrVector<char>(size) {dc=deepc;}
   ~Q3StrVec()  { clear(); }
private:
    Item	 newItem( Item d )	{ return dc ? qstrdup( (const char*)d ) : d; }
    void deleteItem( Item d )	{ if ( dc ) delete[] (char*)d; }
    int	 compareItems( Item s1, Item s2 )
				{ return qstrcmp((const char*)s1,
						(const char*)s2); }
#ifndef QT_NO_DATASTREAM
    QDataStream &read( QDataStream &s, Item &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, Item d ) const
				{ return s << (const char*)d; }
#endif
    bool dc;
};


class Q3StrIVec : public Q3StrVec	// case insensitive string vec
{
public:
    Q3StrIVec() {}
    Q3StrIVec( uint size, bool dc = true ) : Q3StrVec( size, dc ) {}
   ~Q3StrIVec() { clear(); }
private:
    int	 compareItems( Item s1, Item s2 )
				{ return qstricmp((const char*)s1,
						 (const char*)s2); }
};

QT_END_HEADER

#endif // Q3STRVEC_H
