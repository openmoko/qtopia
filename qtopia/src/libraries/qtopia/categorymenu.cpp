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
#include "categorymenu.h"
#include "categoryselect.h"

#include <qstring.h>
#include <qmap.h>

/*!
  \class CategoryMenu
  \brief The CategoryMenu widget aids in filtering records or files by Category.

  The CategoryMenu widget provides a popup menu that will make filtering records
  or files by category much easier.  The widget will lookup the available
  categories for an application, populate the menu, and keep a track of which
  categories are being filtered against.  A set of categories can be tested
  by the isSelected() function to see if a record or file containing those
  categories would be allowed through by the filter.

  \warning Currently this class is not suitable for extending.

  \ingroup qtopiaemb
*/

/*!
  \fn void CategoryMenu::categoryChange()
  This signal is emitted when the user selects a different category in the
  menu, hence changing what records or files should be selected.
*/

/*!
  Creates a new CategoryMenu with \a parent and \a name.  The menu will be
  populated with the available categories for \a application.

  If \a globals is TRUE then it will also poplulate the menu with the
  global categories.
*/
CategoryMenu::CategoryMenu( const QString &application, bool globals,
	QWidget *parent, const char *name) :
    QPopupMenu(parent, name),
	appName(application),
	includeGlobal(globals)
{
    currentMid = 1;
    reload();
    connect(this, SIGNAL(activated(int)), this, SLOT(mapMenuId(int)));
}

/*!
  Destroys a CategoryMenu.
*/
CategoryMenu::~CategoryMenu( )
{
}

/*!
  Repopulates the widget's list of available categories.
*/
void CategoryMenu::reload()
{
    clear();
    Categories c;

    c.load(categoryFileName());

    void qpe_translateLabels(QStringList& strs);
    QString qpe_translateLabel(const QString&);

    QStringList sl = c.labels(appName, includeGlobal);
    qpe_translateLabels(sl);
    int mid = 1;

    insertItem(qpe_translateLabel("_All"), mid);
    mid++;
    insertItem(qpe_translateLabel("_Unfiled"), mid);
    mid++;

    for (QStringList::Iterator it = sl.begin();
	    it != sl.end(); ++it ) {
	int cid = c.id(appName, *it);
	insertItem(*it, mid);
	menuToId.insert(mid, cid);
	idToMenu.insert(cid, mid);
	mid++;
    }

    setItemChecked(currentMid, TRUE );
}

/*!
  \internal
*/
void CategoryMenu::mapMenuId(int id)
{
    if (id == currentMid)
	return;
    setItemChecked( currentMid, FALSE );
    setItemChecked( id, TRUE );
    currentMid = id;

    emit categoryChange();
}

/*!
  Returns TRUE if a record or file with the set of category ids \a cUids
  is allowed by the current selection in the CategoryMenu.
  Otherwise returns FALSE.
*/
bool CategoryMenu::isSelected(const QArray<int> &cUids) const
{
    if (currentMid == 1)
	return TRUE;

    if (currentMid == 2 && cUids.count() == 0)
	return TRUE;

    if (cUids.contains(menuToId[currentMid]))
	return TRUE;

    return FALSE;
}

/*!
  Sets the menu to have \a newCatUid as the currently selected Category.
*/
void CategoryMenu::setCurrentCategory( int newCatUid )
{
    if (!idToMenu.contains(newCatUid))
	return;

    mapMenuId(idToMenu[newCatUid]);
}

/*!
  Sets the menu to allow all category sets.
*/
void CategoryMenu::setCurrentCategoryAll( )
{
    mapMenuId(1);
}

/*!
  Sets the menu to allow only empty category sets.
*/
void CategoryMenu::setCurrentCategoryUnfiled( )
{
    mapMenuId(2);
}
