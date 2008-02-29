/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "colorselector.h"
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>

class ColorSelectorPrivate
{
public:
    QRgb palette[256];
    QColor defCol;
    int highlighted;
    bool pressed;
};

ColorSelector::ColorSelector( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    setBackgroundColor( white );
    d = new ColorSelectorPrivate;
    d->highlighted = -2;
    d->pressed = FALSE;
    int idx = 0;
    for( int ir = 0x0; ir <= 0xff; ir+=0x55 ) {
	for( int ig = 0x0; ig <= 0xff; ig+=0x55 ) {
	    for( int ib = 0x0; ib <= 0xff; ib+=0x55 ) {
		d->palette[idx]=qRgb( ir, ig, ib );
		idx++;
	    }
	}
    }

    bool changed = TRUE;
    while ( changed ) {
	changed = FALSE;
	int i = 0;
	QColor col( d->palette[i] );
	while ( i < idx-1 ) {
	    QColor ncol( d->palette[i+1] );
	    int h1, s1, v1;
	    int h2, s2, v2;
	    col.hsv( &h1, &s1, &v1 );
	    ncol.hsv( &h2, &s2, &v2 );
	    if ( h1*255+v1 > h2*255+v2 ) {
		QRgb tmp = d->palette[i];
		d->palette[i] = d->palette[i+1];
		d->palette[i+1] = tmp;
		changed = TRUE;
	    }
	    col = ncol;
	    i++;
	}
    }
    setMinimumSize( 65, 65 );
    setFocus();
}

ColorSelector::~ColorSelector()
{
    delete d;
}

QSize ColorSelector::sizeHint() const
{
    return QSize( 12*8+1, 12*8+1 + (d->defCol.isValid()?16:0) );
}

void ColorSelector::setDefaultColor( const QColor &c )
{
    d->defCol = c;
    if ( !col.isValid() && d->defCol.isValid() )
	d->highlighted = -1;
    update();
}

const QColor &ColorSelector::defaultColor() const
{
    return d->defCol;
}

void ColorSelector::setColor( const QColor &c )
{
    col = c;
    if ( QColor(d->defCol) == c ) {
	d->highlighted = -1;
	update();
	return;
    }

    int r = c.red();
    int g = c.green();
    int b = c.blue();
    int rd = (QColor(d->palette[0]).red() - r);
    int gd = (QColor(d->palette[0]).green() - g);
    int bd = (QColor(d->palette[0]).blue() - b);
    int bestCol = 0;
    int bestDiff = rd*rd + gd*gd + bd*bd;
    for ( int i = 0; i < 8*8; i++ ) {
	if ( QColor(d->palette[i]) == c ) {
	    d->highlighted = i;
	    update();
	    return;
	} else {
	    rd = (QColor(d->palette[i]).red() - r);
	    gd = (QColor(d->palette[i]).green() - g);
	    bd = (QColor(d->palette[i]).blue() - b);
	    int diff = rd*rd + gd*gd + bd*bd;
	    if ( diff < bestDiff ) {
		bestDiff = diff;
		bestCol = i;
	    }
	}
    }

    // We don't have the exact color specified, so we'll pick the closest.
    // Not really optimal, but better than settling for the default.
    d->highlighted = bestCol;
    update();
}

void ColorSelector::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    int cw = (width()-1)/8;
    int ch = (height()-1-(d->defCol.isValid()?16:0))/8;
    int idx = 0;
    for ( int y = 0; y < 8; y++ ) {
	for ( int x = 0; x < 8; x++ ) {
	    p.fillRect( x*cw+1, y*ch+1, cw-1, ch-1, QColor(d->palette[idx]) );
	    if ( idx == d->highlighted )
		p.drawRect( x*cw, y*ch, cw+1, ch+1 );
	    idx++;
	}
    }
    if ( d->defCol.isValid() ) {
	p.fillRect( 1, 8*ch+1, width()-2, height()-(8*ch+1), d->defCol );
	if ( d->highlighted == -1 )
	    p.drawRect( 0, 8*ch, width(), height()-(8*ch) );
	if ( qGray(d->defCol.rgb()) < 128 )
	    p.setPen( white );
	p.drawText( 0, 8*ch+1, width(), height()-(8*ch+1), AlignCenter, tr("Default") );
    }
}

void ColorSelector::mousePressEvent( QMouseEvent *me )
{
    d->pressed = TRUE;
    mouseMoveEvent( me );
}

void ColorSelector::mouseMoveEvent( QMouseEvent *me )
{
    int cw = (width()-1)/8;
    int ch = (height()-1-(d->defCol.isValid()?16:0))/8;
    int row = (me->pos().y()-1)/ch;
    int col = (me->pos().x()-1)/cw;
    int old = d->highlighted;
    if ( row >=0 && row < 8 && col >= 0 && col < 8 )
	d->highlighted = row*8+col;
    else if ( d->defCol.isValid() && row >=8 && me->pos().y() < height() )
	d->highlighted = -1;
    else
	d->highlighted = -2;

    if ( old != d->highlighted ) {
	repaint( rectOfColor( old ) );
	repaint( rectOfColor( d->highlighted ) );
    }
}

void ColorSelector::mouseReleaseEvent( QMouseEvent * )
{
    if ( !d->pressed )
	return;
    if ( d->highlighted >= 0 ) {
	col = QColor(d->palette[d->highlighted]);
	emit selected( col );
    } else if ( d->highlighted == -1 ) {
	col = d->defCol;
	emit selected( col );
    }
}

void ColorSelector::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
	case Key_Left:
	    if ( d->highlighted > 0 || d->highlighted == -1 ) {
		int old = d->highlighted;
		if ( d->highlighted == -1 )
		    d->highlighted = 63;
		else
		    d->highlighted--;
		repaint( rectOfColor( old ) );
		repaint( rectOfColor( d->highlighted ) );
	    }
	    break;
	case Key_Right:
	    if ( d->highlighted < 64 && d->highlighted != -1 ) {
		int old = d->highlighted;
		d->highlighted++;
		if ( d->highlighted == 64 )
		    d->highlighted = -1;
		repaint( rectOfColor( old ) );
		repaint( rectOfColor( d->highlighted ) );
	    }
	    break;
	case Key_Up:
	    if ( d->highlighted > 7 || d->highlighted == -1 ) {
		int old = d->highlighted;
		if ( d->highlighted == -1 )
		    d->highlighted = 63;
		else
		    d->highlighted-=8;
		repaint( rectOfColor( old ) );
		repaint( rectOfColor( d->highlighted ) );
	    }
	    break;
	case Key_Down:
	    if ( d->highlighted != -1 ) {
		int old = d->highlighted;
		d->highlighted+=8;
		if ( d->highlighted > 63 )
		    d->highlighted = -1;
		repaint( rectOfColor( old ) );
		repaint( rectOfColor( d->highlighted ) );
	    }
	    break;
	case Key_Space:
	case Key_Return:
	    if ( d->highlighted >= 0 ) {
		col = QColor(d->palette[d->highlighted]);
		emit selected( col );
	    } else if ( d->highlighted == -1 ) {
		col = d->defCol;
		emit selected( col );
	    }
	    topLevelWidget()->hide();
	    break;
	case Key_Escape:
	    topLevelWidget()->hide();
	    break;
    }
}

void ColorSelector::showEvent( QShowEvent *e )
{
    QWidget::showEvent(e);
    d->pressed = FALSE;
}

QRect ColorSelector::rectOfColor( int idx ) const
{
    QRect r;
    int cw = (width()-1)/8;
    int ch = (height()-1-(d->defCol.isValid()?16:0))/8;
    if ( idx >= 0 ) {
	int row = idx/8;
	int col = idx%8;
	r = QRect( col*cw, row*ch, cw+1, ch+1 );
    } else if ( idx == -1 ) {
	r = QRect( 0, ch*8, width(), height()-ch*8 );
    }

    return r;
}

//===========================================================================

class ColorButtonPrivate
{
public:
    QPopupMenu *popup;
    ColorSelector *picker;
};

ColorButton::ColorButton( QWidget *parent, const char *name )
    : QPushButton( parent, name )
{
    init();
}

ColorButton::ColorButton( const QColor &c, QWidget *parent, const char *name )
    : QPushButton(parent,name), col(c)
{
    init();
}

ColorButton::~ColorButton()
{
    delete d;
}

void ColorButton::init()
{
    d = new ColorButtonPrivate;
    d->popup = new QPopupMenu( this );
    d->picker = new ColorSelector( d->popup );
    d->popup->insertItem( d->picker );
    setPopup( d->popup );
    connect( d->picker, SIGNAL(selected(const QColor &)),
	    this, SIGNAL(selected(const QColor &)) );
    connect( d->picker, SIGNAL(selected(const QColor &)),
	    this, SLOT(colorSelected(const QColor &)) );
}

void ColorButton::setColor( const QColor &c )
{
    col = c;
    d->picker->setColor( c );
    update();
}

void ColorButton::setDefaultColor( const QColor &c )
{
    d->picker->setDefaultColor( c );
}

const QColor &ColorButton::defaultColor() const
{
     return d->picker->defaultColor();
}

void ColorButton::colorSelected( const QColor &c )
{
    d->popup->hide();
    col = c;
}

void ColorButton::drawButtonLabel( QPainter *p )
{
    QRect r = style().pushButtonContentsRect( this );
    if ( isDown() || isOn() ){
	int sx = 0;
	int sy = 0;
	style().getButtonShift(sx, sy);
	r.moveBy( sx, sy );
    }
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    int dx = style().menuButtonIndicatorWidth( height() );
    style().drawArrow( p, DownArrow, FALSE, x+w-dx, y+2, dx-4, h-4,
			colorGroup(), isEnabled() );
    w -= dx;
    if ( col.isValid() )
	p->fillRect( x+2, y+2, w-6, h-4 , col );
    else if ( defaultColor().isValid() )
	p->fillRect( x+2, y+2, w-6, h-4 , defaultColor() );
}

