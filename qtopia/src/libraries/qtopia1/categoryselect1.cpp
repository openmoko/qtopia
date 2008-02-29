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
    The \a parent and \a name parameters are the standard Qt parent parameters.

  First availability: Qtopia 1.6
*/
CategorySelect::CategorySelect( const QArray<int> &vlCats, const QString &appName,
				 const QString &visibleName, bool allCategories, QWidget *parent,
				const char *name , int width) :
    QHBox( parent, name ),
    cmbCat( 0 ),
    cmdCat( 0 ),
    d( 0 )
{
    init(width);
    setAllCategories(allCategories);
    setCategories( vlCats, appName, visibleName );
}

/*!
  Returns the type of widget as a CategorySelect::SelectorWidget.

  First availability: Qtopia 1.6
*/

CategorySelect::SelectorWidget CategorySelect::widgetType() const
{
    if ( cmbCat )
	return ComboBox;
    return ListView;
}

/*!
  Returns TRUE if the current category contains any categories in \a cats.

  First availability: Qtopia 1.6
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

/*!
  \fn void CategorySelect::editCategoriesClicked()

  This signal is emitted when the user wishes to edit the
  categories, just prior to showing the dialog allowing that.

  \sa CategoryEdit.

  First availability: Qtopia 1.6
*/
