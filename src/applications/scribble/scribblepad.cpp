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

#include "scribblepad.h"

#include <qpe/qpeapplication.h>

#include <qcombobox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpen.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>

//
//
//
ScribblePad::ScribblePad(QWidget *parent, const char *name):
    QWidget(parent, name)
{
    setMouseTracking(FALSE);

    buffer = new QPixmap;
    pen = new QPen;
    brush = new QBrush(QBrush::SolidPattern);

    pen->setWidth(3);

    resize(qApp->desktop()->width(), qApp->desktop()->height());
}

//
//
//
ScribblePad::~ScribblePad(void)
{
    delete brush;
    delete pen;
    delete buffer;
}

//
//
//
void
ScribblePad::clear(void)
{
    buffer->fill();
    bitBlt(this, 0, 0, buffer);
}

//
//
//
void
ScribblePad::changeColor(const QColor &c)
{
    pen->setColor(c);
    brush->setColor(c);
}

//
//
//
void
ScribblePad::changePenSize(const QString &string)
{
    pen->setWidth(string.toInt());
}

//
//
//
void
ScribblePad::mousePressEvent(QMouseEvent *e)
{
    last = e->pos();
    mouseDown = TRUE;
}
void
ScribblePad::mouseReleaseEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
    mouseDown = FALSE;
}

//
//
//
void
ScribblePad::mouseMoveEvent(QMouseEvent *e)
{
    QPainter	painter;

    if (mouseDown) {
	//
	// XXXX for some reason drawLine() gives ugly results for
	// short slow movements.  Figure out WTF is going on here
	// and fix.
	//
	painter.begin(buffer);
	painter.setBrush(QBrush::SolidPattern);
	painter.setPen(*pen);

        painter.drawRect(
	    e->pos().x() - (pen->width() / 2),
	    e->pos().y() - (pen->width() / 2),
	    pen->width(), pen->width());
	painter.end();

	//
	// XXXX setup bounding rectangle so we don't blit the
	// entire screen.
	//
	bitBlt(this, 0, 0, buffer);

	last = e->pos();
    }
}

//
//
//
void
ScribblePad::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    if ((buffer->size() != e->size())) {
	bitBlt(this, 0, 0, buffer);
    }
}

//
//
//
void
ScribblePad::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    bitBlt(this, 0, 0, buffer);
}

//
//
//
QPixmap
ScribblePad::picture(void) const
{
    return *buffer;
}

//
//
//
void
ScribblePad::setPicture(const QPixmap &p)
{
    delete buffer;
    buffer = new QPixmap(p);
}
