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

#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>
#include <qtopia/calc/display.h>
#include <qtopia/calc/engine.h>
#include <qpixmap.h>
#include <qwhatsthis.h>

// Lcd display class
MyLcdDisplay::MyLcdDisplay(QWidget *p,const char *n,WFlags f)
    :QScrollView(p,n,f) {
    QWhatsThis::add( this, tr("Displays the current input or result") );
    lcdPixmap = 0;
    lcdPainter = 0;
    setHScrollBarMode(QScrollView::AlwaysOff);
    setVScrollBarMode(QScrollView::AlwaysOff);

    QFont smallFont(QApplication::font());
    smallFont.setPointSize( 10 );
    smallFont.setBold(TRUE);
    
    bigFont.setFamily("helvetica");
    bigFont.setPointSize(24);
    setFont(bigFont);

    if ( !Global::mousePreferred() ) {
        setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));
        setFrameStyle(NoFrame);
        viewport()->setBackgroundMode(PaletteBase);
    } else { //pda or touchscreen
        setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Fixed));
    }
}
MyLcdDisplay::~MyLcdDisplay() {
    delete lcdPixmap;
    delete lcdPainter;
}

QSize MyLcdDisplay::sizeHint() const
{
    if (!Global::mousePreferred())
        return QScrollView::sizeHint();
    else
        return QSize(-1,fontMetrics().lineSpacing()+2*frameWidth());
}

void MyLcdDisplay::readStack() {
    bool stateOk = systemEngine->checkState();

    if (!lcdPixmap){
	lcdPixmap = new QPixmap(visibleWidth(),2000);
        lcdPixmap->fill();
    }
    if (!lcdPainter)
	lcdPainter = new QPainter();

    static QPixmap save = *lcdPixmap;
    
    bitBlt(lcdPixmap, 0,0, &save);
    lcdPainter->begin(lcdPixmap,this);
    lcdPainter->setPen(Qt::black);

    verticalOffset=0; // top margin
    int horizontalOffset = 10; // right margin
    if (!stateOk) {
        lcdPainter->setFont(smallFont);
	lcdPainter->drawText(5,5,visibleWidth() - horizontalOffset,20,Qt::AlignRight,systemEngine->errorString);
	verticalOffset=25;
    } else {
        lcdPainter->setFont(bigFont);
        if (Global::mousePreferred()) {
            QPixmap *tmp;
            if (systemEngine->dStack.isEmpty())
                return;
            int myoffset = 10;
            tmp = systemEngine->dStack.top()->draw();

            int drawPoint = QMAX(visibleWidth() - tmp->width(),0);
            int srcStart = QMAX(tmp->width() - visibleWidth(),0);
            lcdPainter->drawPixmap(drawPoint - myoffset,verticalOffset,*tmp,srcStart);
            verticalOffset += tmp->height();
        }
#ifdef QTOPIA_PHONE
        else {
            niStack = new QStack<QString>();
            ndStack = new QStack<Data>();
            while (!systemEngine->iStack.isEmpty())
                niStack->push(systemEngine->iStack.pop());
            while (!systemEngine->dStack.isEmpty())
                ndStack->push(systemEngine->dStack.pop());
            dataLeft = 1;
            while (!niStack->isEmpty() || !ndStack->isEmpty()) {
                horizontalOffset = 10;
                horizontalOffset = drawNextItem(horizontalOffset,TRUE);
            }

            delete niStack;
            delete ndStack;
        }
#endif
    }

    if (!Global::mousePreferred())
        resizeContents(visibleWidth(),verticalOffset);
    updateContents(0,0,visibleWidth(),verticalOffset);
    lcdPainter->end();
    if (!Global::mousePreferred())
        ensureVisible(visibleWidth(),verticalOffset);
}

#ifdef QTOPIA_PHONE
int MyLcdDisplay::drawNextItem(int hoffset,bool newline) {
    QPixmap *tmp;
    int myoffset = hoffset;
    if (!niStack->isEmpty() && *(niStack->top()) == "Open brace impl") { // No tr
	Instruction *tmpi = systemEngine->resolve(*(niStack->top()));
	tmp = tmpi->draw();
	systemEngine->iStack.push(niStack->pop());
	myoffset += drawNextItem(hoffset,FALSE);
    } else if (dataLeft && !ndStack->isEmpty()) {
	tmp = ndStack->top()->draw();
	systemEngine->dStack.push(ndStack->pop());
	dataLeft--;
    } else if (!niStack->isEmpty()) {
	Instruction *tmpi = systemEngine->resolve(*(niStack->top()));
	tmp = tmpi->draw();
	dataLeft = tmpi->argCount - 1;
	systemEngine->iStack.push(niStack->pop());
	if (tmpi->name != "EvaluateLine") // No tr
	    myoffset += drawNextItem(hoffset,FALSE);
    } else
	return 0;
    int drawPoint = QMAX(visibleWidth() - tmp->width(),0);
    int srcStart = QMAX(tmp->width() - visibleWidth(),0);
    lcdPainter->drawPixmap(drawPoint - myoffset,verticalOffset,*tmp,srcStart);
    if (newline)
	verticalOffset += tmp->height();
    return myoffset + tmp->width();
}
#endif

void MyLcdDisplay::drawContents(QPainter *p,int x,int y, int w, int h) {
    if (lcdPixmap)
	p->drawPixmap(x,y,*lcdPixmap,x,y,w,h);
}

