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

#include <qtopia/categories.h>

/*!
  \overload

  Returns list of ids associated with the group.

  First availability: Qtopia 2.0
*/
QArray<int> CategoryGroup::ids() const
{
    QArray<int> a(mIdLabelMap.count());
    int i=0;
    for (QMap<int, QString>::ConstIterator it = mIdLabelMap.begin(); it!=mIdLabelMap.end(); ++it)
	a[i++] = it.key();
    return a;
}


/*!
  \overload

  Returns list of category ids associated with the application \a app,
  and, if \a includeGlobal is true, the global category ids.

  First availability: Qtopia 2.0
*/
QArray<int> Categories::ids( const QString & app, bool includeGlobal ) const
{
    QArray<int> a;
    if ( includeGlobal )
	a = mGlobalCats.ids();
    if ( !app.isEmpty() ) {
	QArray<int> b = mAppCats[app].ids();
	int s = a.size();
	a.resize(s+b.size());
	memcpy(a.data()+s,b.data(),b.size()*sizeof(int));
    }
    return a;
}

