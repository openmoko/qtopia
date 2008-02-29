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

#include "griditemtable.h"
#include "griditem.h"


/*!
  \internal
  \class GridItemTable

  \brief   GridItemTable stores pointers to GridItem objects in a row * column grid format.

  \mainclass
  The size of the table (rows or columns) grows as GridItem objects are added to it.

  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
  \sa GridItem
*/


/*!
  \internal
  \fn GridItemTable::GridItemTable()
  Constructs an empty table containing neither rows nor columns.
*/
GridItemTable::GridItemTable() : mTopRow(-1), mTopColumn(-1) {}

/*!
  \internal
  \fn bool GridItemTable::add(GridItem *item)
  Adds the given object and returns true if there was no previous item at item's row and column,
  or else returns false if an item has already been placed at item's row and column, in which
  case the item is not added. Note that this method may affect the value(s) returned by
  \l{function}{topRow()} and \l{function}{topColumn()}.
*/
bool GridItemTable::add(GridItem *item)
{
    Key key(item->row(),item->column());

    if ( map.contains(key) ) {
        return false;
    }

    map[key] = item;

    if ( item->row() > mTopRow ) {
        mTopRow = item->row();
    }
    if ( item->column() > mTopColumn ) {
        mTopColumn = item->column();
    }

    return true;
}

/*!
  \internal
  \fn GridItem *GridItemTable::item(const Key &key) const
  Returns the item at Key's row and column, or 0 if none exists.
*/
GridItem *GridItemTable::item(const Key &key) const
{
    if ( !(map.contains(key)) ) {
        return 0;
    }
    return map[key];
}

/*!
  \internal
  \fn GridItem *GridItemTable::item(int row,int col) const
  Returns the item at the given row and column, or 0 if none exists.
*/
GridItem *GridItemTable::item(int row,int col) const
{
    Key key(row,col);
    return item(key);
}


