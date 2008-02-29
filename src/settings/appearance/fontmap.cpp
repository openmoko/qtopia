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

#include "fontmap.h"
#include <qtopia/config.h>

FontMap::FontMap()
{
    Config qpecfg("qpe");
    QString fam = qpecfg.readEntry("FontFamily");
    int siz = qpecfg.readNumEntry("FontSize",0);

    Config cfg("FontMap");
    cfg.setGroup("Map");

    int i = 0;
    QString key("Font0");
    while (cfg.hasKey(key)) {
	QStringList fl = cfg.readListEntry(key, ',');
	Font fnt(fl[0], fl[1], fl[2].toInt());
	if ( fam.isEmpty() )
	    fam = fnt.family;
	if ( !siz )
	    siz = fnt.size;
	fontMap.append(fnt);
	i++;
	key = QString("Font%1").arg(i);
    }

    select(fam,siz);
}

void FontMap::select(const QString& family, int size)
{
    int i=0;
    for (QValueList<Font>::ConstIterator it=fontMap.begin(); it!=fontMap.end(); ++it, ++i) {
	if ( (*it).family == family && (*it).size == size ) {
	    sel = i;
	    return;
	}
    }
    sel = 0;
}

void FontMap::select(const QString& name)
{
    int i=0;
    for (QValueList<Font>::ConstIterator it=fontMap.begin(); it!=fontMap.end(); ++it, ++i) {
	if ( (*it).name.lower() == name.lower() ) {
	    sel = i;
	    return;
	}
    }
    sel = 0;
}

void FontMap::read()
{
    Config config("qpe");
    config.setGroup( "Appearance" );
    read(config);
}

void FontMap::read(Config& config)
{
    QString fam = config.readEntry("FontFamily", "helvetica");
    int siz = config.readNumEntry("FontSize", 12);
    select(fam,siz);
}

void FontMap::write()
{
    Config config("qpe");
    config.setGroup( "Appearance" );
    write(config);
}

void FontMap::write(Config& config)
{
    Font f = current();
    config.writeEntry("FontFamily", f.family);
    config.writeEntry("FontSize", f.size);
}
