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

#ifndef PICKER_H
#define PICKER_H

#include <QWidget>
#include <QAbstractScrollArea>
#include "inputmatchglobal.h"

class QTOPIA_INPUTMATCH_EXPORT Picker : public QAbstractScrollArea
{
    Q_OBJECT
public:
    Picker(QWidget *parent=0);
    ~Picker();

    bool filterKey(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);
    bool filterMouse(const QPoint &, int, int = 0);

    void setMicroFocus( int x, int y );
    void setAppFont(const QFont &f);

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
    void paintEvent(QPaintEvent* event);
    void drawFrame(QPainter *p);
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);

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
};

#endif

