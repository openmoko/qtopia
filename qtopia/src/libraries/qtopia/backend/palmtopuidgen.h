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

#ifndef QTPALMTOP_UIDGEN_H
#define QTPALMTOP_UIDGEN_H

#include <qtopia/qpeglobal.h>
#include <time.h>
#include <qmap.h>

namespace Qtopia {


class QTOPIA_EXPORT UidGen
{
public:
    enum Type { Qtopia, PalmtopCenter };

    UidGen() : type( Qtopia ), sign( -1 ), ids()
{
#ifdef QTOPIA_DESKTOP
    type = PalmtopCenter;
    sign = 1;
#endif
}
    UidGen( Type t ) : type(t), sign(1), ids()
{
    if ( t == Qtopia )
	sign = -1;
}

    virtual ~UidGen() { }

    int generate() const
{
    int id = sign * (int) ::time(NULL);

    // Don't allow ID's in the range -9 to 9.
    // This provides room for sentinels and such.
    while ( (id < 9 && id > -9) || ids.contains( id ) ) {
	id += sign;
	
	// check for overflow cases; if so, wrap back to beginning of
	// set ( -1 or 1 )
	if ( sign == -1 && id > 0 || sign == 1 && id < 0 )
	    id = sign;
    }
    return id;
}

    void store(int id) { ids.insert(id, TRUE); }
    bool isUnique(int id) const { return (!ids.contains(id)); }

private:
    Type type;
    int sign;
    QMap<int, bool> ids;

};

}

#define QTOPIA_DEFINED_UIDGEN
#include <qtopia/qtopiawinexport.h>

#endif
