/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "charlist.h"
#include <qapplication.h>
#include <qpainter.h>
#include <qtimer.h>

CharList::CharList(QWidget *parent, const char *name, WFlags f)
    : QWidget(parent, name, f | WStyle_Customize | WStyle_Tool)
{
    setBackgroundMode( QWidget::PaletteBase );

    fm = new QFontMetrics( font() );
    cellHeight = fm->lineSpacing() + 3;
    cellWidth = fm->width("W")+1;

    resize(50, cellHeight);
}

CharList::~CharList()
{
}


void CharList::setMicroFocus( int x, int y )
{
    QPoint p;
    if (y < 30 + appFont.pointSize() + height() + 5)
	p = QPoint(x, y+2);
    else
	p = QPoint(x, y-appFont.pointSize()-4-height());

    if (p.x() + width() > qApp->desktop()->width())
	p.setX(qApp->desktop()->width()-width());

    move(p);
}

void CharList::setChars(const QStringList &ch)
{
    chars = ch;
    int c = chars.findIndex(current);
    current = c >= 0 ? ch[c] : ch[0];
    cellWidth = 5;
    for (int i=0; i < (int)chars.count(); i++) {
	if (fm->width(chars[i]) > cellWidth)
	    cellWidth = fm->width(chars[i]);
    }
    resize(cellWidth*chars.count()+2, height());
    update();
}

void CharList::setCurrent(const QString &ch)
{
    current = ch;
    update();
}

void CharList::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBrush(NoBrush);
    p.drawRect(0, 0, width(), height());
    int pos = 1;
    for (int i=0; i < (int)chars.count(); i++) {
	QString ch = chars[i];
	if (current == ch) {
	    p.setPen(colorGroup().base());
	    p.fillRect(pos, 1, cellWidth, height()-2, colorGroup().text());
	} else {
	    p.setPen(colorGroup().text());
	    p.fillRect(pos, 1, cellWidth, height()-2, colorGroup().base());
	}
	int cw = fm->width(ch);
	p.drawText(pos+(cellWidth-cw)/2, fm->ascent()+1, ch);
	pos += cellWidth;
    }
}

