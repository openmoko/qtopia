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

#include "unikeyboard.h"

#include <qtopia/fontmanager.h>

#include <qpainter.h>
#include <qfontmetrics.h>
#include <qcombobox.h>
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
#include <qwindowsystem_qws.h>
#endif

static int nw = 8;

typedef struct BlockMap {
    ushort start;
    ushort stop;
    const char *name;
};

//# Start Code; Block Name

static const BlockMap blockMap[] = 
{
{0x0000, 0x007F, QT_TRANSLATE_NOOP("UniKeyboard", "Basic Latin")},
{0x0080, 0x00FF, QT_TRANSLATE_NOOP("UniKeyboard", "Latin-1 Supplement")},
{0x0100, 0x017F, QT_TRANSLATE_NOOP("UniKeyboard", "Latin Extended-A")},
{0x0180, 0x024F, QT_TRANSLATE_NOOP("UniKeyboard", "Latin Extended-B")},
{0x0250, 0x02AF, QT_TRANSLATE_NOOP("UniKeyboard", "IPA Extensions")},
{0x02B0, 0x02FF, QT_TRANSLATE_NOOP("UniKeyboard", "Spacing Modifier Letters")},
{0x0300, 0x036F, QT_TRANSLATE_NOOP("UniKeyboard", "Combining Diacritical Marks")},
{0x0370, 0x03FF, QT_TRANSLATE_NOOP("UniKeyboard", "Greek")},
{0x0400, 0x04FF, QT_TRANSLATE_NOOP("UniKeyboard", "Cyrillic")},
{0x0530, 0x058F, QT_TRANSLATE_NOOP("UniKeyboard", "Armenian")},
{0x0590, 0x05FF, QT_TRANSLATE_NOOP("UniKeyboard", "Hebrew")},
{0x0600, 0x06FF, QT_TRANSLATE_NOOP("UniKeyboard", "Arabic")},
{0x0700, 0x074F, QT_TRANSLATE_NOOP("UniKeyboard", "Syriac")},
{0x0780, 0x07BF, QT_TRANSLATE_NOOP("UniKeyboard", "Thaana")},
{0x0900, 0x097F, QT_TRANSLATE_NOOP("UniKeyboard", "Devanagari")},
{0x0980, 0x09FF, QT_TRANSLATE_NOOP("UniKeyboard", "Bengali")},
{0x0A00, 0x0A7F, QT_TRANSLATE_NOOP("UniKeyboard", "Gurmukhi")},
{0x0A80, 0x0AFF, QT_TRANSLATE_NOOP("UniKeyboard", "Gujarati")},
{0x0B00, 0x0B7F, QT_TRANSLATE_NOOP("UniKeyboard", "Oriya")},
{0x0B80, 0x0BFF, QT_TRANSLATE_NOOP("UniKeyboard", "Tamil")},
{0x0C00, 0x0C7F, QT_TRANSLATE_NOOP("UniKeyboard", "Telugu")},
{0x0C80, 0x0CFF, QT_TRANSLATE_NOOP("UniKeyboard", "Kannada")},
{0x0D00, 0x0D7F, QT_TRANSLATE_NOOP("UniKeyboard", "Malayalam")},
{0x0D80, 0x0DFF, QT_TRANSLATE_NOOP("UniKeyboard", "Sinhala")},
{0x0E00, 0x0E7F, QT_TRANSLATE_NOOP("UniKeyboard", "Thai")},
{0x0E80, 0x0EFF, QT_TRANSLATE_NOOP("UniKeyboard", "Lao")},
{0x0F00, 0x0FFF, QT_TRANSLATE_NOOP("UniKeyboard", "Tibetan")},
{0x1000, 0x109F, QT_TRANSLATE_NOOP("UniKeyboard", "Myanmar")},
{0x10A0, 0x10FF, QT_TRANSLATE_NOOP("UniKeyboard", "Georgian")},
{0x1100, 0x11FF, QT_TRANSLATE_NOOP("UniKeyboard", "Hangul Jamo")},
{0x1200, 0x137F, QT_TRANSLATE_NOOP("UniKeyboard", "Ethiopic")},
{0x13A0, 0x13FF, QT_TRANSLATE_NOOP("UniKeyboard", "Cherokee")},
{0x1400, 0x167F, QT_TRANSLATE_NOOP("UniKeyboard", "Unified Canadian Aboriginal Syllabics")},
{0x1680, 0x169F, QT_TRANSLATE_NOOP("UniKeyboard", "Ogham")},
{0x16A0, 0x16FF, QT_TRANSLATE_NOOP("UniKeyboard", "Runic")},
{0x1780, 0x17FF, QT_TRANSLATE_NOOP("UniKeyboard", "Khmer")},
{0x1800, 0x18AF, QT_TRANSLATE_NOOP("UniKeyboard", "Mongolian")},
{0x1E00, 0x1EFF, QT_TRANSLATE_NOOP("UniKeyboard", "Latin Extended Additional")},
{0x1F00, 0x1FFF, QT_TRANSLATE_NOOP("UniKeyboard", "Greek Extended")},
{0x2000, 0x206F, QT_TRANSLATE_NOOP("UniKeyboard", "General Punctuation")},
{0x2070, 0x209F, QT_TRANSLATE_NOOP("UniKeyboard", "Superscripts and Subscripts")},
{0x20A0, 0x20CF, QT_TRANSLATE_NOOP("UniKeyboard", "Currency Symbols")},
{0x20D0, 0x20FF, QT_TRANSLATE_NOOP("UniKeyboard", "Combining Marks for Symbols")},
{0x2100, 0x214F, QT_TRANSLATE_NOOP("UniKeyboard", "Letterlike Symbols")},
{0x2150, 0x218F, QT_TRANSLATE_NOOP("UniKeyboard", "Number Forms")},
{0x2190, 0x21FF, QT_TRANSLATE_NOOP("UniKeyboard", "Arrows")},
{0x2200, 0x22FF, QT_TRANSLATE_NOOP("UniKeyboard", "Mathematical Operators")},
{0x2300, 0x23FF, QT_TRANSLATE_NOOP("UniKeyboard", "Miscellaneous Technical")},
{0x2400, 0x243F, QT_TRANSLATE_NOOP("UniKeyboard", "Control Pictures")},
{0x2440, 0x245F, QT_TRANSLATE_NOOP("UniKeyboard", "Optical Character Recognition")},
{0x2460, 0x24FF, QT_TRANSLATE_NOOP("UniKeyboard", "Enclosed Alphanumerics")},
{0x2500, 0x257F, QT_TRANSLATE_NOOP("UniKeyboard", "Box Drawing")},
{0x2580, 0x259F, QT_TRANSLATE_NOOP("UniKeyboard", "Block Elements")},
{0x25A0, 0x25FF, QT_TRANSLATE_NOOP("UniKeyboard", "Geometric Shapes")},
{0x2600, 0x26FF, QT_TRANSLATE_NOOP("UniKeyboard", "Miscellaneous Symbols")},
{0x2700, 0x27BF, QT_TRANSLATE_NOOP("UniKeyboard", "Dingbats")},
{0x2800, 0x28FF, QT_TRANSLATE_NOOP("UniKeyboard", "Braille Patterns")},
{0x2E80, 0x2EFF, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Radicals Supplement")},
{0x2F00, 0x2FDF, QT_TRANSLATE_NOOP("UniKeyboard", "Kangxi Radicals")},
{0x2FF0, 0x2FFF, QT_TRANSLATE_NOOP("UniKeyboard", "Ideographic Description Characters")},
{0x3000, 0x303F, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Symbols and Punctuation")},
{0x3040, 0x309F, QT_TRANSLATE_NOOP("UniKeyboard", "Hiragana")},
{0x30A0, 0x30FF, QT_TRANSLATE_NOOP("UniKeyboard", "Katakana")},
{0x3100, 0x312F, QT_TRANSLATE_NOOP("UniKeyboard", "Bopomofo")},
{0x3130, 0x318F, QT_TRANSLATE_NOOP("UniKeyboard", "Hangul Compatibility Jamo")},
{0x3190, 0x319F, QT_TRANSLATE_NOOP("UniKeyboard", "Kanbun")},
{0x31A0, 0x31BF, QT_TRANSLATE_NOOP("UniKeyboard", "Bopomofo Extended")},
{0x3200, 0x32FF, QT_TRANSLATE_NOOP("UniKeyboard", "Enclosed CJK Letters and Months")},
{0x3300, 0x33FF, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Compatibility")},
{0x3400, 0x4DB5, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Unified Ideographs Extension A")},
{0x4E00, 0x9FFF, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Unified Ideographs")},
{0xA000, 0xA48F, QT_TRANSLATE_NOOP("UniKeyboard", "Yi Syllables")},
{0xA490, 0xA4CF, QT_TRANSLATE_NOOP("UniKeyboard", "Yi Radicals")},
{0xAC00, 0xD7A3, QT_TRANSLATE_NOOP("UniKeyboard", "Hangul Syllables")},
{0xD800, 0xDB7F, QT_TRANSLATE_NOOP("UniKeyboard", "High Surrogates")},
{0xDB80, 0xDBFF, QT_TRANSLATE_NOOP("UniKeyboard", "High Private Use Surrogates")},
{0xDC00, 0xDFFF, QT_TRANSLATE_NOOP("UniKeyboard", "Low Surrogates")},
{0xE000, 0xF8FF, QT_TRANSLATE_NOOP("UniKeyboard", "Private Use")},
{0xF900, 0xFAFF, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Compatibility Ideographs")},
{0xFB00, 0xFB4F, QT_TRANSLATE_NOOP("UniKeyboard", "Alphabetic Presentation Forms")},
{0xFB50, 0xFDFF, QT_TRANSLATE_NOOP("UniKeyboard", "Arabic Presentation Forms-A")},
{0xFE20, 0xFE2F, QT_TRANSLATE_NOOP("UniKeyboard", "Combining Half Marks")},
{0xFE30, 0xFE4F, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Compatibility Forms")},
{0xFE50, 0xFE6F, QT_TRANSLATE_NOOP("UniKeyboard", "Small Form Variants")},
{0xFE70, 0xFEFE, QT_TRANSLATE_NOOP("UniKeyboard", "Arabic Presentation Forms-B")},
{0xFF00, 0xFFEF, QT_TRANSLATE_NOOP("UniKeyboard", "Halfwidth and Fullwidth Forms")},
{0xFFF0, 0xFFFD, QT_TRANSLATE_NOOP("UniKeyboard", "Specials")},
{0xFFFF, 0xFFFF, 	0} };


UniScrollview::UniScrollview(QWidget* parent, const char* name, int f) :
    QScrollView(parent, name, f), selRow(-1), selCol(-1)
{
    //    smallFont.setRawName( "-adobe-courier-medium-r-normal--10-100-75-75-m-60-iso8859-1" ); //######
    smallFont = QFont( "Helvetica", 8 );
    QFontMetrics sfm( smallFont );
    xoff = sfm.width( "AAA" );
#ifdef QWS
    setFont( FontManager::unicodeFont( FontManager::Fixed ) );
#endif
    QFontMetrics fm( font() );
    cellsize = fm.lineSpacing() + 2;
    verticalScrollBar()->setLineStep(cellsize);
    
    viewport()->setBackgroundMode( QWidget::PaletteBase );
}


void UniScrollview::resizeEvent(QResizeEvent *e)
{
    nw = ( e->size().width() - xoff ) / cellsize;
    nw -= nw % 8;
    nw = QMAX(nw, 8);
    resizeContents( cellsize*nw, cellsize*65536/nw );
    
    QScrollView::resizeEvent(e);
}

void UniScrollview::contentsMousePressEvent(QMouseEvent* e)
{
    if ( e->x() < xoff || e->x() > xoff + nw*cellsize )
	return;
    int row = e->y()/cellsize;
    int col = (e->x()-xoff)/cellsize;
    int u = row*nw+col;
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    uint code = 0;
    if ( u >= 'a' && u <= 'z' ) {
	code = u - 'a' + Key_A;
    }
    emit key( u, code, 0, true, false );
    emit key( u, code, 0, false, false );
#endif
    selRow = row;
    selCol = col;
    updateContents( xoff+col*cellsize, row*cellsize, cellsize, cellsize );
}


void UniScrollview::contentsMouseReleaseEvent(QMouseEvent*)
{
    int row = selRow;
    int col = selCol;
    selRow = -1;
    selCol = -1;
    updateContents( xoff+col*cellsize, row*cellsize, cellsize, cellsize );
}

void UniScrollview::scrollTo( int unicode )
{
    int row = unicode / nw;
    setContentsPos( 0, row*cellsize );
}


void UniScrollview::drawContents( QPainter *p, int /*cx*/, int cy, int /*cw*/, int ch )
{
    QFontMetrics fm = fontMetrics();
    int row = cy / cellsize;
    int y = row*cellsize;
    while ( y < cy+ch ) {
	p->drawLine( xoff, y, xoff+nw*cellsize, y );
	if ( row*nw%16 == 0 ) {
	    p->setFont( smallFont );
	    QString s;
	    s.sprintf( "%03X", row*nw/16 ); 
	    p->drawText( 0, y, xoff, cellsize, AlignLeft, s );
	    p->setFont( font() );
	}
	for ( int i = 0; i < nw; i++ ) {
	    p->drawLine( xoff+i*cellsize, y, xoff+i*cellsize, y+cellsize );
	    QChar u = row*nw + i;
	    if ( row == selRow && i == selCol ) {
		p->setPen( white );
		p->fillRect( xoff+i*cellsize+1, y+1, cellsize-1, cellsize-1, black );
	    } else {
		p->setPen( colorGroup().text() );
		p->fillRect( xoff+i*cellsize+1, y+1, cellsize-1, cellsize-1, colorGroup().base() );
	    }
	    if ( fm.inFont( u ) )
		p->drawText( xoff+i*cellsize, y, cellsize, cellsize, AlignCenter,
			     u );
	}
	p->drawLine( xoff+nw*cellsize, y, xoff+nw*cellsize, y+cellsize );
	row++;
	y += cellsize;
    }
}




UniKeyboard::UniKeyboard(QWidget* parent, const char* name, int f )
    : QFrame( parent, name, f )
{
    setFrameStyle( NoFrame );
    sv = new UniScrollview( this );
    cb = new QComboBox( FALSE, this );
    cb->setSizeLimit( 5 );
    currentBlock = 0;
    QFontMetrics fm = sv->fontMetrics();
    cbmap = new int[sizeof(blockMap)/sizeof(blockMap[0])];
    for ( int i = 0; blockMap[i].name; i++ ) {
	bool any=FALSE;
	for ( int c=blockMap[i].start; !any && c<=blockMap[i].stop; c++ )
	    any = fm.inFont(QChar(c));
	if ( any ) {
	    cbmap[cb->count()]=i;
	    cb->insertItem( tr(blockMap[i].name) );
	}
    }
    connect( cb, SIGNAL( activated(int)), this, SLOT( handleCombo(int)) );
    connect( sv, SIGNAL( contentsMoving(int,int)), this, SLOT( svMove(int,int)) );
    connect( sv, SIGNAL( key(ushort,ushort,ushort,bool,bool)),
	    this, SIGNAL( key(ushort,ushort,ushort,bool,bool)) );
}

UniKeyboard::~UniKeyboard()
{
    delete [] cbmap;
}

void UniKeyboard::resizeEvent(QResizeEvent *)
{
    int d = frameWidth();
    cb->setGeometry( d, d, width()-2*d, cb->sizeHint().height() );
    sv->setGeometry( d, cb->height()+d, width()-2*d, height()-cb->height()-2*d );
}

void UniKeyboard::svMove( int /*x*/, int y )
{
    int cs = sv->cellSize();
    int u = ((y+cs-1)/cs) * nw;
    int i = currentBlock;
    while ( i > 0 && blockMap[i].start > u ) {
       i--;
    }
    while ( blockMap[i+1].name && blockMap[i+1].start < u ) {
	i++;
    }
    if ( i != currentBlock ) {
	currentBlock = i;
	int closest = 0;
	for (int ind=0; ind <= cb->count() && blockMap[cbmap[ind]].start < blockMap[i].stop; ind++) {
	    closest = ind;
	    if ( cbmap[ind] == i ) {
		break;
	    }
	}
	cb->setCurrentItem( closest );
    }
}

void UniKeyboard::handleCombo( int i )
{
    currentBlock = cbmap[i];
    sv->scrollTo( blockMap[currentBlock].start );
}

void UniKeyboard::resetState()
{
}

QSize UniKeyboard::sizeHint() const
{
    return QSize( 240, 2+sv->cellSize()*4+cb->sizeHint().height() );
}
