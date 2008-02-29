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

#include "data.h"

#include <qapplication.h>

Data::Data(){
    clear();
    cache = 0;
};
Data::~Data() {
    delete cache;
};

bool Data::push(char,bool) {
    return FALSE;
}

bool Data::del() {
    return TRUE;
}

void Data::clear() {}

QString Data::getType() {
    return QString("NONE");
}

QString Data::getFormattedOutput() {
    return formattedOutput;
}

QPixmap* Data::draw() {
    QString currentOutput = getFormattedOutput();
    if (cachedOutput != currentOutput) {
	delete cache;
	cache = 0;
    }
    if (!cache) {
	QFont myFont = QFont( "helvetica", 24, QFont::Bold );
	cachedOutput = currentOutput;
	QRect bRect = QFontMetrics(myFont).boundingRect(0,0,240,20,Qt::AlignLeft,cachedOutput);
	cache = new QPixmap(bRect.width(),bRect.height());
	cache->fill();
	QPainter p(cache);
	p.setPen(Qt::black);
	p.setFont(myFont);
	p.drawText(0,0,bRect.width(),bRect.height(),Qt::AlignRight,cachedOutput);
    }
    return cache;
}
