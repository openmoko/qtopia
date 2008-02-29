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
#include "TEHistory.h"

// A non-arbitrary long scroll (a rewrite of something infinite)

class HistoryLine {
public:
    HistoryLine() : len(0), ch(0) {}
    void set(const ca* c, int l)
    {
	if ( ch )
	    delete [] ch;
	ch = new ca[l];
	memcpy(ch,c,sizeof(*ch)*l);
	len = l;
    }
    int len;
    ca* ch;
};

class HistoryScrollData {
public:
    HistoryScrollData(int mx) :
	count(0), off(0), max(mx), arr(new HistoryLine[mx])
    {
	on = FALSE;
    }
    ~HistoryScrollData()
    {
	delete [] arr;
    }
    const HistoryLine* line(int i) const
    {
	return &arr[(i+off)%max];
    }
    void addLine(const ca* c, int l)
    {
	if ( count == max ) {
	    off=(off+1)%max;
	    arr[off].set(c,l);
	} else {
	    arr[count++].set(c,l);
	}
    }
    int count;
    bool on;
private:
    int off, max;
    HistoryLine* arr;
};


HistoryScroll::HistoryScroll()
{
    d = new HistoryScrollData(400);
}

HistoryScroll::~HistoryScroll()
{
    delete d;
}
 
void HistoryScroll::setScroll(bool on)
{
    if ( d->on != on )
	d->on = on;
}

bool HistoryScroll::hasScroll() const
{
    return d->on;
}

int HistoryScroll::getLines() const
{
    if (!hasScroll()) return 0;
    return d->count;
}

int HistoryScroll::getLineLen(int lineno) const
{
    if (!hasScroll()) return 0;
    return d->line(lineno)->len;
}

void HistoryScroll::getCells(int lineno, int colno, int count, ca res[]) const
{
    memcpy(res,d->line(lineno)->ch+colno,count*sizeof(ca));
}

ca HistoryScroll::getCell(int lineno, int colno) const
{
    return *(d->line(lineno)->ch+colno);
}


void HistoryScroll::addLine(ca text[], int count)
{
    if (!hasScroll()) return;
    d->addLine(text,count);
}
