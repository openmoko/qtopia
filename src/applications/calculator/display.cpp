/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include <qtopiaapplication.h>

//#include <QBitmap>
//#include <QPixmap>
#include <QWhatsThis>
#include <QDebug>

#include "display.h"
#include "engine.h"

// Lcd display class
MyLcdDisplay::MyLcdDisplay(QWidget *p)
    :QScrollArea(p) {
    setWhatsThis( tr("Displays the current input or result") );
    lcdPixmap = 0;
    lcdPainter = 0;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFont smallFont(QApplication::font());
    smallFont.setPointSize( 10 );
    smallFont.setBold(true);

    bigFont.setFamily("dejavu");
    bigFont.setPointSize(11);
    setFont(bigFont);

    if ( !Qtopia::mousePreferred() ) {
        setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));
        viewport()->setBackgroundRole( QPalette::NoRole );
        setFrameStyle(NoFrame);
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
    if (!Qtopia::mousePreferred())
        return QScrollArea::sizeHint();
    else
        return QSize(-1,fontMetrics().lineSpacing()+2*frameWidth());
}

static const int pmHeight = 500;

void MyLcdDisplay::readStack() {
    bool stateOk = systemEngine->checkState();

    int visibleWidth = viewport()->size().width();

    if (!lcdPixmap) {
        lcdPixmap = new QBitmap(visibleWidth-3,pmHeight);
    }
    lcdPixmap->clear();

    if ( lcdPainter )
       delete lcdPainter;
    lcdPainter = new QPainter();

    lcdPainter->begin(lcdPixmap);
    lcdPainter->setPen(Qt::color1);

    verticalOffset=0; // top margin
    int horizontalOffset = 10; // right margin
    if (!stateOk) {
        lcdPainter->setFont(smallFont);
        lcdPainter->drawText(5,5,visibleWidth - horizontalOffset,20,Qt::AlignRight,systemEngine->errorString);
        verticalOffset=25;
    } else {
        lcdPainter->setFont(bigFont);
        if (Qtopia::mousePreferred()) {
            QPixmap *tmp;
            if (systemEngine->dStack.isEmpty())
                return;
            int myoffset = 10;
            tmp = systemEngine->dStack.top()->draw();

            int drawPoint = qMax(visibleWidth - tmp->width(),0);
            int srcStart = qMax(tmp->width() - visibleWidth,0);

            lcdPainter->drawPixmap(drawPoint - myoffset,verticalOffset,*tmp,srcStart, 0, -1, -1);
            verticalOffset += tmp->height();
        }
#ifdef QTOPIA_PHONE
        else {
            niStack = new QStack<QString*>();
            ndStack = new QStack<Data*>();
            while (!systemEngine->iStack.isEmpty())
                niStack->push(systemEngine->iStack.pop());
            while (!systemEngine->dStack.isEmpty())
                ndStack->push(systemEngine->dStack.pop());
            dataLeft = 1;
            while (!niStack->isEmpty() || !ndStack->isEmpty()) {
                horizontalOffset = 10;
                horizontalOffset = drawNextItem(horizontalOffset,true, visibleWidth);
            }

            delete niStack;
            delete ndStack;
        }

#endif
    }
    lcdPainter->end();

    QWidget *w = viewport();
    //w->resize(visibleWidth, verticalOffset);
    //w->update(0, 0, visibleWidth, verticalOffset);
    w->update();
}

#ifdef QTOPIA_PHONE
int MyLcdDisplay::drawNextItem(int hoffset,bool newline, int visibleWidth) {
    QPixmap *tmp;
    int myoffset = hoffset;
    if (!niStack->isEmpty() && *(niStack->top()) == "Open brace impl") { // No tr
        Instruction *tmpi = systemEngine->resolve(*(niStack->top()));
        tmp = tmpi->draw();
        systemEngine->iStack.push(niStack->pop());
        myoffset += drawNextItem(hoffset,false, visibleWidth);
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
            myoffset += drawNextItem(hoffset,false, visibleWidth);
    } else
        return 0;
    int drawPoint = qMax(visibleWidth - tmp->width(),0);
    int srcStart = qMax(tmp->width() - visibleWidth,0);
    lcdPainter->drawPixmap(drawPoint - myoffset,verticalOffset,*tmp,srcStart, 0, -1, -1);
    if (newline) {
        verticalOffset += tmp->height();
        if ( lcdPixmap->height() - verticalOffset < 50 ) {
            lcdPainter->end();
            QBitmap *old = lcdPixmap;
            lcdPixmap = new QBitmap( visibleWidth-3, old->height()+pmHeight );
            lcdPainter->begin(lcdPixmap);
            lcdPainter->setPen(Qt::color1);
            delete old;
        }
    }
    return myoffset + tmp->width();
}
#endif

void MyLcdDisplay::paintEvent(QPaintEvent *pe)
{
    Q_UNUSED(pe);
    QPainter wPainter(viewport());

    if (lcdPixmap) {
        int offset = qMin(0, viewport()->size().height() - verticalOffset);
        wPainter.drawPixmap(1,offset,*lcdPixmap);
    }
}
