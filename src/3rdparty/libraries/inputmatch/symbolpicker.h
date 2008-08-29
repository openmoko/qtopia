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

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "picker.h"
#include <qbitmap.h>

class QTOPIA_INPUTMATCH_EXPORT SymbolPicker : public Picker
{
    Q_OBJECT
public:
    SymbolPicker(QWidget *parent=0);
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

