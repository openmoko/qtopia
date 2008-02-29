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
#include <qframe.h>
#include "pickboardcfg.h"
#include "pickboardpicks.h"

class QTimer;

class KeyboardConfig : public DictFilterConfig
{
public:
    KeyboardConfig(PickboardPicks* p) : DictFilterConfig(p), backspaces(0) { nrows = 1; }
    virtual void generateText(const QString &s);
    void decBackspaces() { if (backspaces) backspaces--; }
    void incBackspaces() { backspaces++; }
    void resetBackspaces() { backspaces = 0; }
private:
    int backspaces;
};


class KeyboardPicks : public PickboardPicks
{
    Q_OBJECT
public:
    KeyboardPicks(QWidget* parent=0, const char* name=0, WFlags f=0)
	: PickboardPicks(parent, name, f), dc(0) { }
    KeyboardPicks::~KeyboardPicks();
    void initialise();
    virtual QSize sizeHint() const;
    KeyboardConfig *dc;
};

class Keyboard : public QFrame
{
    Q_OBJECT
public:
    Keyboard( QWidget* parent=0, const char* name=0, WFlags f=0 );

    void resetState();

    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent* e);
    void timerEvent(QTimerEvent* e);
    void drawKeyboard( QPainter &p, int key = -1 );

    void setMode(int mode) { useOptiKeys = mode; }

    QSize sizeHint() const;

signals:
    void key( ushort scancode, ushort unicode, ushort modifiers, bool, bool );

private slots:
    void repeat();
    
private:
    int getKey( int &w, int j = -1 );
    void clearHighlight();

    uint shift:1;
    uint lock:1;
    uint ctrl:1;
    uint alt:1;
    uint useLargeKeys:1;
    uint useOptiKeys:1;
    
    int pressedKey;

    KeyboardPicks *picks;

    int keyHeight;
    int defaultKeyWidth;
    int xoffs;

    int unicode;
    int qkeycode;
    int modifiers;

    int pressTid;
    bool pressed;

    QTimer *repeatTimer;
};

