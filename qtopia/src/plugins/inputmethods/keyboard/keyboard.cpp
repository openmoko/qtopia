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

#include "keyboard.h"

#include <qtopia/global.h>
#include <qtopia/resource.h>

#ifdef QWS
#include <qwindowsystem_qws.h>
#endif
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qtimer.h>
#include <ctype.h>


#define USE_SMALL_BACKSPACE

enum { BSCode = 0x80, TabCode, CapsCode, RetCode, 
       ShiftCode, CtrlCode, AltCode, SpaceCode, BackSlash,
       UpCode, LeftCode, DownCode, RightCode, Blank, Expand,
       Opti, ResetDict,
       Divide, Multiply, Add, Subtract, Decimal, Equal,
       Percent, Sqrt, Inverse, Escape };

typedef struct SpecialMap {
    int qcode;
    ushort unicode;
    const char * label;
    const char * picName;
    QPixmap pic;
};


static SpecialMap specialM[] = {
#ifdef USE_SMALL_BACKSPACE
    {	Qt::Key_Backspace,	8,	"<",     "backspace_small", QPixmap() },
#else
    {	Qt::Key_Backspace,	8,	"<",     "backspace", QPixmap() },
#endif
    {	Qt::Key_Tab,		9,	"Tab",   NULL, QPixmap() }, // No tr
    {	Qt::Key_CapsLock,	0xffff,	"Caps",  NULL, QPixmap() }, // No tr
    {	Qt::Key_Return,		13,	"Ret",   NULL, QPixmap() }, // No tr
    {	Qt::Key_Shift,		0xffff,	"Shift", NULL, QPixmap() }, // No tr
    {	Qt::Key_Control,	0xffff,	"Ctrl",  NULL, QPixmap() }, // No tr
    {	Qt::Key_Alt,		0xffff,	"Alt",   NULL, QPixmap() }, // No tr
    {	Qt::Key_Space,		' ',	"",      NULL, QPixmap() },
    {	BackSlash,		43,	"\\",    NULL, QPixmap() },

    // Need images?
    {	Qt::Key_Up,		0xffff,	"^",     "uparrow", QPixmap() },
    {	Qt::Key_Left,		0xffff,	"<",     "leftarrow", QPixmap() },
    {	Qt::Key_Down,		0xffff,	"v",     "downarrow", QPixmap() },
    {	Qt::Key_Right,		0xffff,	">",     "rightarrow", QPixmap() },
    {	Qt::Key_Insert,		0xffff,	"I",     "insert", QPixmap() },
    {	Qt::Key_Home,		0xffff,	"H",     "home", QPixmap() },
    {	Qt::Key_PageUp,		0xffff,	"U",     "pageup", QPixmap() },
    {	Qt::Key_End,		0xffff,	"E",     "end", QPixmap() },
    {	Qt::Key_Delete,		0xffff,	"X",     "delete", QPixmap() },
    {	Qt::Key_PageDown,	0xffff,	"D",     "pagedown", QPixmap() },
    {	Blank,			0,	" ",     NULL, QPixmap() },
    {	Expand,			0xffff,	"->",    "expand", QPixmap() },
    {	Opti,			0xffff,	"#",     NULL, QPixmap() },
    {	ResetDict,		0xffff,	"R",     NULL, QPixmap() },
   
    // number pad stuff
    {	Divide,			0,	"/",     NULL, QPixmap() },
    {	Multiply,		0,	"*",     NULL, QPixmap() },
    {	Add,			0,	"+",     NULL, QPixmap() },
    {	Subtract,		0,	"-",     NULL, QPixmap() },
    {	Decimal,		0,	".",     NULL, QPixmap() },
    {	Equal,			0,	"=",     NULL, QPixmap() },
    {	Percent,		0,	"%",     NULL, QPixmap() },
    {	Sqrt,			0,	"^1/2",	 NULL, QPixmap() },
    {	Inverse,		0,	"1/x",	 NULL, QPixmap() },

    {	Escape,			27,	"ESC",	 "escape", QPixmap() },
    {	0,			0,	NULL,	 NULL, QPixmap()}
};



Keyboard::Keyboard(QWidget* parent, const char* name, WFlags f) :
    QFrame(parent, name, f),  shift(FALSE), lock(FALSE), ctrl(FALSE),
    alt(FALSE), useLargeKeys(TRUE), useOptiKeys(0), pressedKey(-1),
    unicode(-1), qkeycode(0), modifiers(0), pressTid(0), pressed(FALSE)
{
    //  setPalette(QPalette(QColor(240,240,230))); // Beige!
    //  setFont( QFont( "Helvetica", 8 ) );
//    setPalette(QPalette(QColor(200,200,200))); // Gray
    setPalette(QPalette(QColor(220,220,220))); // Gray

    int specialIndex=0;
    while  (specialM[specialIndex].qcode != 0 ){
	if (specialM[specialIndex].picName != NULL) {
	    QString pName = QString("keyboard/") + specialM[specialIndex].picName;
	    // qDebug("Keyboard pix %s:%d, has name %s",  specialM[specialIndex].label, specialIndex, pName.latin1());
	    specialM[specialIndex].pic = Resource::loadPixmap(pName); 
	}else{
	    // qDebug("Skipping keyboard pix for %s:%d", specialM[specialIndex].label, specialIndex);
	}
        specialIndex++;
    }

    picks = new KeyboardPicks( this );
// under Win32 we may not have smallsmooth font 
#ifndef Q_OS_WIN32
    picks->setFont( QFont( "smallsmooth", 9 ) );
    setFont( QFont( "smallsmooth", 9 ) );
#endif
    picks->initialise();
    QObject::connect( picks, SIGNAL(key(ushort,ushort,ushort,bool,bool) ),
            this, SIGNAL(key(ushort,ushort,ushort,bool,bool)) );

    repeatTimer = new QTimer( this );
    connect( repeatTimer, SIGNAL(timeout()), this, SLOT(repeat()) );
}

void Keyboard::resizeEvent(QResizeEvent*)
{
    int ph = picks->sizeHint().height();
    picks->setGeometry( 0, 0, width(), ph );
    keyHeight = (height()-ph)/5;
    int nk;
    if ( useOptiKeys ) {
	nk = 15;
    } else if ( useLargeKeys ) {
	nk = 15;
    } else {
	nk = 19;
    }
    defaultKeyWidth = width()/nk;
    xoffs = (width()-defaultKeyWidth*nk)/2;
}


KeyboardPicks::~KeyboardPicks()
{
    delete dc;
}

void KeyboardPicks::initialise()
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
    mode = 0;
    dc = new KeyboardConfig(this);
    configs.append(dc);
}

QSize KeyboardPicks::sizeHint() const
{
    return QSize(240,fontMetrics().lineSpacing());
}


void  KeyboardConfig::generateText(const QString &s)
{
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
		  int i;
    for ( i=0; i<(int)backspaces; i++) {
			    parent->emitKey( 0, Qt::Key_Backspace, 0, true, false );
	      parent->emitKey( 0, Qt::Key_Backspace, 0, false, false );
    }
    for ( i=0; i<(int)s.length(); i++) {
	      uint code = 0;
	      if ( s[i].unicode() >= 'a' && s[i].unicode() <= 'z' ) {
	          code = s[i].unicode() - 'a' + Key_A;
	      }
	      parent->emitKey( s[i].unicode(), code, 0, true, false );
	      parent->emitKey( s[i].unicode(), code, 0, false, false );
    }
    parent->emitKey( 0, Qt::Key_Space, 0, true, false );
    parent->emitKey( 0, Qt::Key_Space, 0, false, false );
    backspaces = 0;
#endif
}


//PC keyboard layout and scancodes

/*
  Format: length, code, length, code, ..., 0
  
  length is measured in half the width of a standard key.
  If code < 0x80, code gives the ASCII value of the key
  
  If code >= 0x80, the key is looked up in specialM[].
  
 */

static const uchar * const keyboard_opti[5] = { 
    (const uchar *const) "\001\223\003\240\002\20\002\41\002\26\002\62\002\56\002\45\002\54\003\200\001\223\002\226\002\235\002\234\002\236",
    (const uchar *const) "\001\223\003\201\004\207\002\30\002\24\002\43\004\207\003\203\001\223\006\002\002\065",
    (const uchar *const) "\001\223\003\202\002\60\002\37\002\23\002\22\002\36\002\21\002\55\003\203\001\223\006\005\002\055",
    (const uchar *const) "\001\223\003\205\004\207\002\27\002\61\002\40\004\207\003\204\001\223\006\010\002\014",
    (const uchar *const) "\001\223\003\206\002\44\002\31\002\57\002\42\002\46\002\25\002\207\003\204\001\223\002\013\002\064\002\015\002\230"
};


static const uchar * const keyboard_standard[5] = {

#ifdef USE_SMALL_BACKSPACE
    (const uchar *const)"\002\240\002`\0021\0022\0023\0024\0025\0026\0027\0028\0029\0020\002-\002=\002\200\002\223\002\215\002\216\002\217",
#else
    (const uchar *const)"\002\051\0021\0022\0023\0024\0025\0026\0027\0028\0029\0020\002-\002=\004\200\002\223\002\215\002\216\002\217",
#endif
    //~ + 123...+ BACKSPACE //+ INSERT + HOME + PGUP

    (const uchar *const)"\003\201\002q\002w\002e\002r\002t\002y\002u\002i\002o\002p\002[\002]\002\\\001\224\002\223\002\221\002\220\002\222",
    //TAB + qwerty..  + backslash //+ DEL + END + PGDN

    (const uchar *const)"\004\202\002a\002s\002d\002f\002g\002h\002j\002k\002l\002;\002'\004\203",
    //CAPS + asdf.. + RETURN 

    (const uchar *const)"\005\204\002z\002x\002c\002v\002b\002n\002m\002,\002.\002/\005\204\002\223\002\223\002\211",
    //SHIFT + zxcv... //+ UP

    (const uchar *const)"\003\205\003\206\022\207\003\206\003\205\002\223\002\212\002\213\002\214" 
    //CTRL + ALT + SPACE //+ LEFT + DOWN + RIGHT
    
};


struct ShiftMap {
    char normal;
    char shifted;
};


static const ShiftMap shiftMap[] = {
    { '`', '~' },
    { '1', '!' },
    { '2', '@' },
    { '3', '#' },
    { '4', '$' },
    { '5', '%' },
    { '6', '^' },
    { '7', '&' },
    { '8', '*' },
    { '9', '(' },
    { '0', ')' },
    { '-', '_' },
    { '=', '+' },
    { '\\', '|' },
    { '[', '{' },
    { ']', '}' },
    { ';', ':' },
    { '\'', '"' },
    { ',', '<' },
    { '.', '>' },
    { '/', '?' }
};


static int keycode( int i2, int j, const uchar **keyboard )
{
    if ( j <0 || j >= 5 )
	return 0;
    
    const uchar *row = keyboard[j];

    while ( *row && *row <= i2 ) {
	i2 -= *row;
	row += 2;
    }

    if ( !*row ) return 0;
    
    int k;
    if ( row[1] >= 0x80 ) {
	k = row[1];
    } else {
	k = row[1]+i2/2;
    }
    
    return k;
}


/*
  return scancode and width of first key in row \a j if \a j >= 0,
  or next key on current row if \a j < 0.
  
*/

int Keyboard::getKey( int &w, int j ) {
    static const uchar *row = 0;
    static int key_i = 0;
    static int scancode = 0;
    static int half = 0;
    
    if ( j >= 0 && j < 5 ) {
	if (useOptiKeys)
    	    row = keyboard_opti[j];
	else
    	    row = keyboard_standard[j];
	half=0;
    }

    if ( !row || !*row ) {
	return 0;    
    } else if ( row[1] >= 0x80 ) {
	scancode = row[1];
	w = (row[0] * w + (half++&1)) / 2;
	row += 2;
	return scancode;
    } else if ( key_i <= 0 ) {
	key_i = row[0]/2;
	scancode = row[1];
    }
    key_i--;
    if ( key_i <= 0 )
	row += 2;
    return scancode++;
}
    

void Keyboard::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setClipRect(e->rect());
    drawKeyboard( painter );
    picks->dc->draw( &painter );
}


/*
  Draw the keyboard.

  If key >= 0, only the specified key is drawn.
*/
void Keyboard::drawKeyboard( QPainter &p, int key )
{
    const bool threeD = FALSE;
    const QColorGroup& cg = colorGroup();
    QColor keycolor = // cg.background();
		    	QColor(240,240,230); // Beige!
    QColor keycolor_pressed = cg.mid();
    QColor keycolor_lo = cg.dark();
    QColor keycolor_hi = cg.light();
    QColor textcolor = QColor(0,0,0); // cg.text();

    int margin = threeD ? 1 : 0;
    
//    p.fillRect( 0, , kw-1, keyHeight-2, keycolor_pressed );

    for ( int j = 0; j < 5; j++ ) {
	int y = j * keyHeight + picks->height() + 1;
	int x = xoffs;
	int kw = defaultKeyWidth;
	int k= getKey( kw, j );
	while ( k ) {
	    if ( key < 0 || k == key ) {
		QString s;
		bool pressed = (k == pressedKey);
		bool blank = (k == 0223);
		QPixmap pic;
		
		if ( k >= 0x80 ) {
		    s = specialM[k - 0x80].label;

		    pic = specialM[k - 0x80].pic;
			
		    if ( k == ShiftCode ) {
			pressed = shift;
		    } else if ( k == CapsCode ) {
			pressed = lock;
		    } else if ( k == CtrlCode ) {
			pressed = ctrl;
		    } else if ( k == AltCode ) {
			pressed = alt;
		    } 
		} else {
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
/*
		    s = QChar( shift^lock ? QWSServer::keyMap()[k].shift_unicode : 
			       QWSServer::keyMap()[k].unicode);
*/
		    // ### Fixme, bad code, needs improving, whole thing needs to
		    // be re-coded to get rid of the way it did things with scancodes etc
		    char shifted = k;
		    if ( !isalpha( k ) ) {
			for ( unsigned i = 0; i < sizeof(shiftMap)/sizeof(ShiftMap); i++ )
			    if ( shiftMap[i].normal == k )
				shifted = shiftMap[i].shifted;
		    } else {
			shifted = toupper( k );
		    }
		    s = QChar( shift^lock ? shifted : k );
#endif
		}

		if (!blank) {
		    if ( pressed )
			p.fillRect( x+margin, y+margin, kw-margin, keyHeight-margin-1, keycolor_pressed );
		    else
			p.fillRect( x+margin, y+margin, kw-margin, keyHeight-margin-1, keycolor );

		    if ( threeD ) {
			p.setPen(pressed ? keycolor_lo : keycolor_hi);
			p.drawLine( x, y+1, x, y+keyHeight-2 );
			p.drawLine( x+1, y+1, x+1, y+keyHeight-3 );
			p.drawLine( x+1, y+1, x+1+kw-2, y+1 );
		    } else if ( j == 0 ) {
			p.setPen(pressed ? keycolor_hi : keycolor_lo);
			p.drawLine( x, y, x+kw, y );
		    }

		    // right
		    p.setPen(pressed ? keycolor_hi : keycolor_lo);
		    p.drawLine( x+kw-1, y, x+kw-1, y+keyHeight-2 );

		    if ( threeD ) {
			p.setPen(keycolor_lo.light());
			p.drawLine( x+kw-2, y+keyHeight-2, x+kw-2, y+1 );
			p.drawLine( x+kw-2, y+keyHeight-2, x+1, y+keyHeight-2 );
		    }

		    if (!pic.isNull()) {
			p.drawPixmap( x + 1, y + 2, pic );
		    } else {
    			p.setPen(textcolor);
			p.drawText( x - 1, y, kw, keyHeight-2, AlignCenter, s );
		    }
	    
		    if ( threeD ) {
			p.setPen(keycolor_hi);
			p.drawLine( x, y, x+kw-1, y );
		    }

		    // bottom
		    p.setPen(keycolor_lo);
		    p.drawLine( x, y+keyHeight-1, x+kw-1, y+keyHeight-1 );
	    
		} else {
		    p.fillRect( x, y, kw, keyHeight, cg.background() );
		}
	    }

	    x += kw;
	    kw = defaultKeyWidth;
	    k = getKey( kw );
	}
    }
}


void Keyboard::mousePressEvent(QMouseEvent *e)
{
    clearHighlight(); // typing fast?

    int i2 = ((e->x() - xoffs) * 2) / defaultKeyWidth;
    int j = (e->y() - picks->height()) / keyHeight;

    int k = keycode( i2, j, (const uchar **)((useOptiKeys) ? keyboard_opti : keyboard_standard) );
    bool need_repaint = FALSE;
    unicode = -1;
    qkeycode = 0;
    if ( k >= 0x80 ) {
	if ( k == ShiftCode ) {
	    shift = !shift;
	    need_repaint = TRUE;
	} else if ( k == AltCode ){
	    alt = !alt;
	    need_repaint = TRUE;
	} else if ( k == CapsCode ) {
	    lock = !lock;
	    need_repaint = TRUE;
	} else if ( k == CtrlCode ) {
	    ctrl = !ctrl;
	    need_repaint = TRUE;
	} else if ( k == 0224 /* Expand */ ) {
	    useLargeKeys = !useLargeKeys;
	    resizeEvent(0);
	    repaint( TRUE ); // need it to clear first
	} else if ( k == 0225 /* Opti/Toggle */ ) {
	    useOptiKeys = !useOptiKeys;
	    resizeEvent(0);
	    repaint( TRUE ); // need it to clear first
	} else {
	    qkeycode = specialM[ k - 0x80 ].qcode;
	    unicode = specialM[ k - 0x80 ].unicode;
	}
    } else {
	//due to the way the keyboard is defined, we know that
	//k is within the ASCII range, and can be directly mapped to 
	//a qkeycode; except letters, which are all uppercase
	qkeycode = toupper(k);
	if ( shift^lock ) {
	    if ( !isalpha( k ) ) {
	    for ( unsigned i = 0; i < sizeof(shiftMap)/sizeof(ShiftMap); i++ )
		if ( shiftMap[i].normal == k ) {
		    unicode = shiftMap[i].shifted;
		    qkeycode = unicode;
		    break;
		}
	    } else {
		unicode = toupper( k );
	    }
	} else {
	    unicode = k;
	}
    }
    if  ( unicode != -1 ) {
	if ( ctrl && unicode >= 'a' && unicode <= 'z' )
	    unicode = unicode - 'a'+1;
	modifiers = (shift ? Qt::ShiftButton : 0) | (ctrl ? Qt::ControlButton : 0) |
		  (alt ? Qt::AltButton : 0);
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
	emit key( unicode, qkeycode, modifiers, true, false );
	repeatTimer->start( 500 );
#endif
	need_repaint = shift || alt || ctrl;
	shift = alt = ctrl = FALSE;

	KeyboardConfig *dc = picks->dc;

	if (dc) {
	    if (qkeycode == Qt::Key_Backspace) {
		dc->input.remove(dc->input.last()); // remove last input
		dc->decBackspaces();
	    } else if ( k == 0226 || qkeycode == Qt::Key_Return ||
		        qkeycode == Qt::Key_Space ||
			QChar(unicode).isPunct() ) {
		dc->input.clear();
		dc->resetBackspaces();
	    } else {
		dc->add(QString(QChar(unicode)));
		dc->incBackspaces();
	    }
	}

	picks->repaint();

    }
    pressedKey = k;
    if ( need_repaint ) {
	repaint( FALSE );
    } else {
	QPainter p(this);
	drawKeyboard( p, pressedKey );
    }
    if ( pressTid )
	killTimer(pressTid);
    pressTid = startTimer(80);
    pressed = TRUE;
}


void Keyboard::mouseReleaseEvent(QMouseEvent*)
{
    if ( pressTid == 0 )
	clearHighlight();
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    if ( unicode != -1 ) {
	emit key( unicode, qkeycode, modifiers, false, false );
	repeatTimer->stop();
    }
#endif
    pressed = FALSE;
}

void Keyboard::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == pressTid ) {
	killTimer(pressTid);
	pressTid = 0;
	if ( !pressed )
	    clearHighlight();
    }
}

void Keyboard::repeat()
{
    repeatTimer->start( 150 );
    emit key( unicode, qkeycode, modifiers, true, true );
}

void Keyboard::clearHighlight()
{
    if ( pressedKey >= 0 ) {
	int tmp = pressedKey;
	pressedKey = -1;
	QPainter p(this);
	drawKeyboard( p, tmp );
    }
}


QSize Keyboard::sizeHint() const
{
    QFontMetrics fm=fontMetrics();
    int keyHeight = fm.lineSpacing()+2;

    if (useOptiKeys)
    	keyHeight += 1;
    
    return QSize( 320, keyHeight * 5 + picks->sizeHint().height() + 1 );
}


void Keyboard::resetState()
{
    picks->resetState();
}
