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

#ifndef PICKER_H
#define PICKER_H

#include <qscrollview.h>
#include <qbitmap.h>

class Picker : public QScrollView
{
    Q_OBJECT
public:
    Picker(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~Picker();

    bool filter(int unicode, int keycode, int modifiers,
                            bool isPress, bool autoRepeat);
    void setMicroFocus( int x, int y );
    void setAppFont(const QFont &f) { appFont = f; }

    void setNumRows(int);
    void setNumCols(int);
    void setCellHeight(int);
    void setCellWidth(int);

    int numRows() const { return nRows; }
    int numCols() const { return nCols; }
    int cellHeight() const { return cHeight; }
    int cellWidth() const { return cWidth; }
signals:
    void cellClicked(int row, int col);

protected:
    virtual void drawCell(QPainter *p, int row, int col, bool selected) = 0;

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

    void contentsMouseReleaseEvent(QMouseEvent *e);

    void drawFrame(QPainter *);
    void drawContents(QPainter *p, int, int cy, int, int ch);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

private:
    void updateContentsSize();

    enum FakeGrab
    {
	None,
	OutSide,
	View,
	VScroll,
	HScroll
    };

    FakeGrab grabber;

    int nCols;
    int nRows;
    int cHeight;
    int cWidth;
    int selRow;
    int selCol;
    bool havePress;
    QFont appFont;
};

#endif

