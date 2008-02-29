/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef GRIDITEMTABLE_H
#define GRIDITEMTABLE_H

#include <QMap>
#include <QPair>

class GridItem;


class GridItemTable
{
public:

    typedef QPair<int,int> Key;

    GridItemTable();

    bool add(GridItem *);

    GridItem *getItem(int row,int col) const;

    GridItem *getItem(const Key &) const;

    int getTopRow() const { return topRow; }

    int getTopColumn() const { return topColumn; }


private:

    // Note: operator> and operator< are defined for QPair. The comparison is done on
    // the first value (row). If that ties, a comparison is made using the second value (column).
    QMap<Key,GridItem *> map;

    int topRow;
    int topColumn;
};

#endif
