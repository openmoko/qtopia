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

#include "symbolpicker.h"
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>
#include <qapplication.h>
#include <qwsdisplay_qws.h>
#include <qtimer.h>
#include <qstyle.h>

#define cr_width 12
#define cr_height 12
static unsigned char cr_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x08, 0x02,
    0x0c, 0x02, 0xfe, 0x03, 0x0c, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 };


SymbolPicker::SymbolPicker(QWidget *parent, const char *name, WFlags f)
    : Picker(parent, name, f)
{
    Config cfg(Global::defaultButtonsFile(), Config::File); // No tr
    cfg.setGroup("IMethod"); // No tr

    int charsPerRow = cfg.readNumEntry("picker_width",13);
    setNumCols(charsPerRow);
    chars = cfg.readEntry("picker_chars");
    if ( chars.isEmpty() ) {
	// Default: latin1 non-letter printable characters, and newline
	int ch;
	chars += QChar('\n');
	for (ch='!'; ch<='/'; ch++) chars += QChar(ch);
	for (ch=':'; ch<='@'; ch++) chars += QChar(ch);
	for (ch='['; ch<='`'; ch++) chars += QChar(ch);
	for (ch='{'; ch<='~'; ch++) chars += QChar(ch);
	for (ch=161; ch<=191; ch++) chars += QChar(ch);
	chars += QChar(215);
	chars += QChar(247);
	chars += QChar(0x20A6);
	chars += QChar(0x20A9);
	chars += QChar(0x20AC);
	chars += QChar(0x2022);
    }

    QFont fnt(font());
    int cw=0;
    int tw;
    do {
	if ( cw ) // not first time round
	    fnt.setPointSize(fnt.pointSize()-2);
	QFontMetrics fm(fnt);
	cw = 1;
	for ( int i=0; i<(int)chars.length(); i++ ) {
	    int w = fm.width(chars[i]);
	    if ( w > cw ) cw = w;
	}
	tw = (cw+2)*charsPerRow;
	tw += style().scrollBarExtent().width();
	tw += frameWidth() * 2;
    } while ( tw > qApp->desktop()->width() && fnt.pointSize() > 6 );

    setFont(fnt);

    QFontMetrics fm( font() );
    setCellHeight( fm.lineSpacing() + 3 );
    setCellWidth( QMIN(cw+1, qApp->desktop()->width() / (charsPerRow+1) ));
    verticalScrollBar()->setLineStep(cellHeight());

    setNumRows((chars.length()+charsPerRow-1)/charsPerRow);

    symbols = new QChar* [numRows()];
    int ch=0;
    for (int i = 0; i < numRows(); i++) {
	symbols[i] = new QChar [charsPerRow];
	for (int c = 0; c < charsPerRow; c++)
	    symbols[i][c] = ch < (int)chars.length() ? chars[ch++] : QChar::null;
    }

    connect(this, SIGNAL(cellClicked(int,int)), this, SLOT(sendSymbolForPos(int,int)));
}

SymbolPicker::~SymbolPicker()
{
    for (int i = 0; i < numRows(); i++)
	delete [] symbols[i];
    delete [] symbols;
}

// do press and release for proper click handling.
void SymbolPicker::sendSymbolForPos(int row, int col)
{
    if (symbols[row][col] != QChar::null)
	emit symbolClicked(symbols[row][col].unicode(), 0);
}

void SymbolPicker::drawCell(QPainter *p, int row, int col, bool selected)
{
    QChar u;
    QFontMetrics fm(font());
    if ( selected ) {
	p->setPen( white );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), black );
    } else {
	p->setPen( colorGroup().text() );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), colorGroup().base() );
    }
    if (symbols[row][col] == '\n') {
	QBitmap bm(cr_width, cr_height, cr_bits, TRUE);
	bm.setMask(bm);
	p->drawPixmap((cellWidth()-bm.width())/2,
		(cellHeight()-bm.height())/2, bm);
    } else {
	u = QChar(symbols[row][col]);
	if (!u.isNull() && fm.inFont(u))
	    p->drawText(0, 0, cellWidth(), cellHeight(), AlignCenter, u);
    }
}

