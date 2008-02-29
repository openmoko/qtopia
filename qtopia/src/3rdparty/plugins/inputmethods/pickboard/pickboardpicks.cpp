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
#include "pickboardpicks.h"
#include "pickboardcfg.h"

#include <qtopia/global.h>
#include <qtopia/resource.h>

#include <qpainter.h>
#include <qlist.h>
#include <qbitmap.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qdialog.h>
#include <qscrollview.h>
#include <qpopupmenu.h>
#include <qhbuttongroup.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#ifdef QWS
#include <qwindowsystem_qws.h>
#endif

void PickboardPicks::doMenu()
{
    QWidget* cause = (QWidget*)sender(); // evil

    QPopupMenu popup(this);
    config()->fillMenu(popup);

    QPoint pos = cause->mapToGlobal(cause->rect().topRight());
    QSize sz = popup.sizeHint();
    pos.ry() -= sz.height();
    pos.rx() -= sz.width();
    popup.move(pos);
    config()->doMenu(popup.exec());
}


PickboardPicks::PickboardPicks(QWidget* parent, const char* name, WFlags f ) :
    QFrame(parent,name,f)
{
    configs.setAutoDelete( TRUE );
}

void PickboardPicks::initialise(void)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
    mode = 0;

    DictFilterConfig* dc = new DictFilterConfig(this);
    QStringList sets_a = QStringList::split(' ',tr("ABC DEF GHI JKL MNO PQR STU VWX YZ-'"));
    QStringList sets = QStringList::split(' ',
	tr("ABCÀÁÂÃÄÅÆÇ DEFÐÈÉÊË GHIÌÍÎÏ JKL MNOÑÒÓÔÕÖØ PQRÞ STUßÙÚÛÜ VWX YZ-'Ýÿ"));
    for (QStringList::ConstIterator it = sets.begin(), it_a = sets_a.begin(); it!=sets.end(); ++it,++it_a)
	dc->addSet(*it_a,*it);
    dc->addMode("123");
    dc->addMode("@*!?");
    dc->addMode(tr("KEY"));
    dc->addMode(tr("Space"));
    dc->addMode(tr("Back"));
    dc->addMode(tr("Enter"));
    dc->addMode(tr("Shift"));
    configs.append(dc);

    CharStringConfig* number = new CharStringConfig(this);
    number->addChar("0");
    number->addChar("1");
    number->addChar("2");
    number->addChar("3");
    number->addChar("4");
    number->addChar("5");
    number->addChar("6");
    number->addChar("7");
    number->addChar("8");
    number->addChar("9");
    number->addChar("."); // #### or "," in some locales
    configs.append(number);

    CharConfig* punc = new CharConfig(this);

    punc->addChar(0,"\"");
    punc->addChar(0,"`");
    punc->addChar(0,"'");
    punc->addChar(0,"\253");
    punc->addChar(0,"\273");
    punc->addChar(0,"\277");
    punc->addChar(1,"(");
    punc->addChar(1,")");
    punc->addChar(1,"[");
    punc->addChar(1,"]");
    punc->addChar(1,"{");
    punc->addChar(1,"}");

    punc->addChar(0,"+");
    punc->addChar(0,"-");
    punc->addChar(0,"*");
    punc->addChar(0,"/");
    punc->addChar(0,"=");
    punc->addChar(0,"_");
    punc->addChar(0,"$");
    punc->addChar(0,"&");
    punc->addChar(1,"|");
    punc->addChar(1,"@");
    punc->addChar(1,"\\");
    punc->addChar(1,"#");
    punc->addChar(1,"^");
    punc->addChar(1,"~");
    punc->addChar(1,"<");
    punc->addChar(1,">");

    punc->addChar(0,".");
    punc->addChar(0,"?");
    punc->addChar(0,"!");
    punc->addChar(0,",");
    punc->addChar(0,";");
    punc->addChar(1,":");
    punc->addChar(1,"\267");
    punc->addChar(1,"\277");
    punc->addChar(1,"\241");
    punc->addChar(1,"\367");

    punc->addChar(0,"$");
    punc->addChar(0,"\242");
    punc->addChar(0,"\245");
    punc->addChar(1,"\243");
    punc->addChar(1,"\244");
    punc->addChar(1,"\260");

    configs.append(punc);

    KeycodeConfig* keys = new KeycodeConfig(this);
    keys->addKey(0,Resource::loadPixmap("pickboard/Esc"),Key_Escape);
    keys->addKey(0,Resource::loadPixmap("pickboard/BS"),Key_Backspace);
    keys->addGap(0,10);

    keys->addKey(0,Resource::loadPixmap("pickboard/Ins"),Key_Insert);
    keys->addKey(0,Resource::loadPixmap("pickboard/Home"),Key_Home);
    keys->addKey(0,Resource::loadPixmap("pickboard/PgUp"),Key_PageUp);

    keys->addGap(0,25);
    keys->addKey(0,Resource::loadPixmap("pickboard/Up"),Key_Up);
    keys->addGap(0,15);

    keys->addKey(1,Resource::loadPixmap("pickboard/BackTab"),Key_Tab);
    keys->addGap(1,3);
    keys->addKey(1,Resource::loadPixmap("pickboard/Tab"),Key_Tab);
    keys->addGap(1,10);

    keys->addKey(1,Resource::loadPixmap("pickboard/Del"),Key_Delete);
    keys->addGap(1,2);
    keys->addKey(1,Resource::loadPixmap("pickboard/End"),Key_End);
    keys->addGap(1,3);
    keys->addKey(1,Resource::loadPixmap("pickboard/PgDn"),Key_PageDown);

    keys->addGap(1,10);
    keys->addKey(1,Resource::loadPixmap("pickboard/Left"),Key_Left);
    keys->addKey(1,Resource::loadPixmap("pickboard/Down"),Key_Down);
    keys->addKey(1,Resource::loadPixmap("pickboard/Right"),Key_Right);

    keys->addGap(1,13);
    keys->addKey(1,Resource::loadPixmap("pickboard/Space"),Key_Space);

    keys->addGap(0,10);
    keys->addKey(0,Resource::loadPixmap("pickboard/Enter"),Key_Return);

    configs.append(keys);
}

PickboardPicks::~PickboardPicks()
{
}

QSize PickboardPicks::sizeHint() const
{
    return QSize(240,fontMetrics().lineSpacing()*2+3);
}

void PickboardPicks::drawContents(QPainter* p)
{
    config()->draw(p);
}

void PickboardPicks::mousePressEvent(QMouseEvent* e)
{
    config()->pickPoint(e->pos(),TRUE);
}

void PickboardPicks::mouseDoubleClickEvent(QMouseEvent* e)
{
    config()->pickPoint(e->pos(),TRUE);
}

void PickboardPicks::mouseReleaseEvent(QMouseEvent* e)
{
    config()->pickPoint(e->pos(),FALSE);
}

void PickboardPicks::setMode(int m)
{
    mode = m;
}

void PickboardPicks::resetState()
{
    config()->doMenu(100);
}
