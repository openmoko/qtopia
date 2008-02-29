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

#include <QApplication>
#include <QPalette>
#include <QPainter>
#include "data.h"

Data::Data(){
    clear();
    cache = 0;
};
Data::~Data() {
    delete cache;
};

bool Data::push(char,bool) {
    return false;
}

bool Data::del() {
    return true;
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
    static QFont myFont = QFont( "dejavu", 9, QFont::Bold );
    if (cachedOutput != currentOutput) {
        delete cache;
        cache = 0;
    }
    if (!cache) {
        cachedOutput = currentOutput;
        QRect bRect = QFontMetrics(myFont).boundingRect(0,0,240,20,Qt::AlignLeft,cachedOutput);
        cache = new QPixmap(bRect.size());
        cache->fill(Qt::transparent);
        QPainter p(cache);
        p.setPen(QApplication::palette().color(QPalette::Text));
        p.setFont(myFont);
        p.drawText(0,0,bRect.width(),bRect.height(),Qt::AlignRight,cachedOutput);
    }
    return cache;
}
