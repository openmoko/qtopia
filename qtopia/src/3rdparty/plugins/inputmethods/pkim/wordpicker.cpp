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

#include "wordpicker.h"
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qapplication.h>
#include <qwsdisplay_qws.h>
#include <qtimer.h>
#include <qstyle.h>

WordPicker::WordPicker(QWidget *parent, const char *name, WFlags f)
    : Picker(parent, name, f)
{
    connect(this, SIGNAL(cellClicked(int,int)), this, SLOT(sendWordChoice(int,int)));
}

WordPicker::~WordPicker()
{
}

void WordPicker::setChoices(const QStringList &list)
{
    choices = list;
    if (choices.count() < 1 && isVisible())
	hide();
    setNumRows(choices.count());
    // work out column width.
    int cw=1;
    QFontMetrics fm(font());
    for ( QStringList::Iterator it = choices.begin(); it != choices.end(); ++it ) {
	cw = QMAX(fm.width(*it), cw);
    }
    setCellWidth(cw+4);
    setCellHeight( fm.lineSpacing() + 3 );
}

void WordPicker::sendWordChoice(int row, int)
{
    emit wordChosen(choices[row]);
}

void WordPicker::drawCell(QPainter *p, int row, int, bool selected)
{
    if ( selected ) {
	p->setPen( colorGroup().highlightedText() );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), colorGroup().highlight() );
    } else {
	p->setPen( colorGroup().text() );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), colorGroup().base() );
    }
    p->drawText(2, 0, cellWidth()-4, cellHeight(), AlignCenter, choices[row]);
}

