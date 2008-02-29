/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/categoryselect.h>

bool CategorySelect::containsCategory( const QArray<int>& cats )
{
    if ( cmdCat && cmdCat->isEnabled() ) {
	// Multi select mode
	int noCats = cats.count();
	for ( int i = 0; i < noCats; i++ )
	    if ( currentCategories().contains( cats[i] ) )
		return TRUE;
    } else {
	// Single select mode
	int catId = currentCategory();
	if ( catId == ALL_CATEGORIES_VALUE || cats.contains( catId )
		|| (catId == UNFILED_CATEGORIES_VALUE && cats.isEmpty()) )
	    return TRUE;
    }

    return FALSE;
}

