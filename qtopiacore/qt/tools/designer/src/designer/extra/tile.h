/****************************************************************************
**
** Copyright (C) 2005-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TILE_H
#define TILE_H

#include <qglobal.h>
#include <item.h>

class Tile
{
public:
    enum Type {
        Earth,
        Wall,
        ClosedDoor,
        OpenDoor,
        Floor
    };


    enum TileFlag {
        Default,
        Explored,
        Lit
    };
    Q_DECLARE_FLAGS(TileFlags, TileFlag)

    Tile (Type t = Earth, TileFlags f = Default) : type(t), flags(f) {}
    Type type;
    TileFlags flags;
    QList<const Item *> items;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Tile::TileFlags);

#endif
