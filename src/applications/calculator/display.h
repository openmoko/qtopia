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

#ifndef CALCULATOR_DISPLAY_H
#define CALCULATOR_DISPLAY_H

#include <QStack>
#include <QScrollArea>
#include "data.h"

class MyLcdDisplay:public QScrollArea {
    Q_OBJECT
public:
    MyLcdDisplay(QWidget *p=0);
    ~MyLcdDisplay();

    QSize sizeHint() const;

public slots:
    void readStack();

protected:
    void paintEvent(QPaintEvent *pe);

private:
    QPixmap *lcdPixmap;
    QPainter *lcdPainter;
    int drawNextItem(int,bool,int);
    int dataLeft,verticalOffset;
    QStack<QString*> *niStack;
    QStack<Data*> *ndStack;
    QFont bigFont, smallFont;
};

#endif
