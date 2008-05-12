/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

    GridItem *item(int row,int col) const;

    GridItem *item(const Key &) const;

    int topRow() const { return mTopRow; }

    int topColumn() const { return mTopColumn; }


private:

    // Note: operator> and operator< are defined for QPair. The comparison is done on
    // the first value (row). If that ties, a comparison is made using the second value (column).
    QMap<Key,GridItem *> map;

    int mTopRow;
    int mTopColumn;
};

#endif
