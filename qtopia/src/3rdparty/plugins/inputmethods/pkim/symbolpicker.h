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

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "picker.h"
#include <qbitmap.h>

class SymbolPicker : public Picker
{
    Q_OBJECT
public:
    SymbolPicker(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~SymbolPicker();

signals:
    void symbolClicked(int unicode, int keycode);

protected:
    void drawCell(QPainter *p, int, int, bool);

private slots:
    void sendSymbolForPos(int, int);

private:
    QString chars;
    int mapRows;
    bool havePress;
    QFont appFont;
    QBitmap crBitmap;
    QChar **symbols;
};

#endif

