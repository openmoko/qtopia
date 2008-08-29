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

#include "modepicker.h"
#include <qtopia/inputmatch/pkimmatcher.h>
#include <qtopiaipcenvelope.h>
#include <qsettings.h>
#include <qtopiaapplication.h>
#include <qapplication.h>
#include <qwsdisplay_qws.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <QPainter>

ModePicker::ModePicker(InputMatcherSet *mset, QWidget *parent)
    : Picker(parent), set(mset)
{
    setNumCols(2);
    QStringList potentials = set->guessCompatibleModes();

    updateModeList();
    connect(this, SIGNAL(cellClicked(int,int)), this, SLOT(setModeFor(int,int)));
}

void ModePicker::updateModeList()
{
    list.clear();
    QStringList potentials = set->guessCompatibleModes();

    QStringList::Iterator it;
    for (it = potentials.begin(); it != potentials.end(); it++) {
	if (!set->pixmapFor(*it).isNull())
	    list.append(*it);
    }

    setNumRows(list.count());
    QSize s = set->pixmapFor(list[0]).size();
    if (s.isValid()) {
	setCellWidth(s.width()+4);
	setCellHeight(s.height()+4);
    }
}

ModePicker::~ModePicker()
{
}

void ModePicker::showEvent(QShowEvent *ev)
{
    updateModeList();
    Picker::showEvent(ev);
}

// do press and release for proper click handling.
void ModePicker::setModeFor(int row, int col)
{
    if (col == 0 || !set->pixmapFor(list[row], col==1).isNull())

	emit modeSelected(list[row], col==1);
}

void ModePicker::drawCell(QPainter *p, int row, int col, bool selected)
{
    Q_UNUSED(selected);
    QPixmap pm = set->pixmapFor(list[row], col == 1);
    if (pm.isNull()) {
	p->fillRect(0, 0, cellWidth(), cellHeight(), palette().base());
    } else {
	p->drawPixmap(2, 2, pm, 0, 0, cellWidth()-4, cellHeight()-4);
    }
//    if (selected)
//	p->setPen(QPen(palette().highlight()));
//    else
//	p->setPen(palette().base());
    p->drawRect(0,0,cellWidth(), cellHeight());
    p->drawRect(1,1,cellWidth()-2, cellHeight()-2);
}

