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

/*!
  \overload

  This constructor accepts an array \a vlCats of integers representing
  Categories. \a appName is the application Categories name to filter on.
  \a visibleName is the string used when the name of this
  widget is required to be displayed. \a allCategories sets whether
  the selector offers "All" and "Unfiled" as options. If set to FALSE,
  then the CategorySelect allows the user to select multiple categories. If
  set to TRUE, the CategorySelect allows for only a single category selection.
  \a width is an integer used as the fixed width of the widget.

*/
CategorySelect::CategorySelect( const QArray<int> &vlCats, const QString &appName,
				 const QString &visibleName, bool add, QWidget *parent,
				const char *name , int width) :
    QHBox( parent, name ),
    cmbCat( 0 ),
    cmdCat( 0 ),
    d( 0 )
{
#ifdef QTOPIA_DESKTOP
    // d will get set in the init
    init(width, add);
#else
    init(width);
    setAllCategories(add);
#endif
    setCategories( vlCats, appName, visibleName );
}

/*!
  Returns the type of widget as a CategorySelect::SelectorWidget.
*/

CategorySelect::SelectorWidget CategorySelect::widgetType() const
{
    if ( cmbCat )
	return ComboBox;
    return ListView;
}

/*!
  Returns TRUE if the current category contains any categories in \a cats.
*/
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

