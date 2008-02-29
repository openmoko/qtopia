/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "instruction.h"
#include "engine.h"

#include <QApplication>
#include <QPalette>
#include <QPainter>

Instruction::Instruction() {
    precedence = 0;
    name = type = retType = "NULL";
    argCount = 0;
    cache = 0;
}

Instruction::~Instruction() {
    delete cache;
};
void Instruction::eval(){qWarning("empty instruction called");}

QPixmap* Instruction::draw() {
    if (!cache) {
        QFont myFont = QFont( "dejavu", 9, QFont::Bold );
        QRect bRect = QFontMetrics(myFont).boundingRect(0,0,240,20,Qt::AlignLeft,displayString);
        cache = new QPixmap(bRect.size());
        cache->fill(Qt::transparent);
        QPainter p(cache);
        p.setPen(QApplication::palette().color(QPalette::Text));
        p.setFont(myFont);
        p.drawText(0,0,bRect.width(),bRect.height(),Qt::AlignRight,displayString);
    }
    return cache;
}
