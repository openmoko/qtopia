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

#include <qapplication.h>
#include <qinputdialog.h>
#include <qpainter.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qtimer.h>
#include <qtopia/mstroke/char.h>
#include "pensettingswidget.h"

#define TITLE_WIDTH	30  // ### magic

/*!
  \class QIMPenSettingsWidget pensettingswidget.h
  \brief The QIMPenSettingsWidget class provides an character input panel.

  Draws characters and allows input of characters.
*/

QIMPenSettingsWidget::QIMPenSettingsWidget( QWidget *parent, const char *name )
 : QWidget( parent, name )
{
    charSets.setAutoDelete( TRUE );
    inputStroke = 0;
    outputChar = 0;
    outputStroke = 0;
    mode = Waiting;
    currCharSet = 0;
    readOnly = FALSE;
    strokes.setAutoDelete( TRUE );

    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), SLOT(timeout()));

    setBackgroundColor( qApp->palette().color( QPalette::Active,
                                               QColorGroup::Base ) );
    strokeColor = black;
    setFixedHeight( 75 );
}

void QIMPenSettingsWidget::clear()
{
    timer->stop();
    mode = Waiting;
    QRect r( dirtyRect );
    QIMPenStrokeIterator it( strokes );
    while ( it.current() ) {
	r |= it.current()->boundingRect();
	++it;
    }
    outputChar = 0;
    outputStroke = 0;
    strokes.clear();
    if ( !r.isNull() ) {
	r.moveBy( -2, -2 );
	r.setSize( r.size() + QSize( 4, 4 ) );
	repaint( r );
    } else {
	repaint();
    }
}

void QIMPenSettingsWidget::removeStroke()
{
    QRect r( dirtyRect );
    QIMPenStroke *st = strokes.getFirst();
    QRect strokeRect;
    if ( st )
       strokeRect = st->boundingRect();
    r |= strokeRect;
    strokes.removeFirst();
    if ( !r.isNull() ) {
	r.moveBy( -2, -2 );
	r.setSize( r.size() + QSize( 4, 4 ) );
	repaint( r );
    }
}

void QIMPenSettingsWidget::greyStroke()
{
    QRect r( dirtyRect );
    QIMPenStroke *st = strokes.getLast();
    QRect strokeRect;
    if ( st )
       strokeRect = st->boundingRect();
    r |= strokeRect;
    QColor oldCol = strokeColor;
    strokeColor = gray;
    if ( !r.isNull() ) {
	r.moveBy( -2, -2 );
	r.setSize( r.size() + QSize( 4, 4 ) );
	repaint( r );
    }
    strokeColor = oldCol;
}

/*
  Insert a character set into the list.
*/
void QIMPenSettingsWidget::insertCharSet( QIMPenCharSet *cs, int stretch, int pos )
{
    CharSetEntry *e = new CharSetEntry;
    e->cs = cs;
    e->stretch = stretch;
    if ( pos < 0 )
	pos = charSets.count();
    charSets.insert( pos, e );
    currCharSet = 0;
    emit changeCharSet( currCharSet );
    emit changeCharSet( charSets.at(currCharSet)->cs );
    totalStretch = 0;
    CharSetEntryIterator it( charSets );
    for ( ; it.current(); ++it )
	totalStretch += it.current()->stretch;
    update();
}

/*
  Remove a character set from the list.
*/
void QIMPenSettingsWidget::removeCharSet( int pos )
{
    if ( pos >= 0 && pos < (int)charSets.count() ) {
	charSets.remove( pos );
	currCharSet = 0;
	if ( charSets.count() ) {
	    emit changeCharSet( currCharSet );
	    emit changeCharSet( charSets.at(currCharSet)->cs );
	}
	totalStretch = 0;
	CharSetEntryIterator it( charSets );
	for ( ; it.current(); ++it )
	    totalStretch += it.current()->stretch;
	update();
    }
}

void QIMPenSettingsWidget::changeCharSet( QIMPenCharSet *cs, int pos )
{
    if ( pos >= 0 && pos < (int)charSets.count() ) {
	CharSetEntry *e = new CharSetEntry;
	e->cs = cs;
	e->stretch = charSets.at(pos)->stretch;
	charSets.remove( pos );
	charSets.insert( pos, e );
	if ( pos == currCharSet ) {
	    emit changeCharSet( charSets.at(currCharSet)->cs );
	}
	update();
    }
}

void QIMPenSettingsWidget::clearCharSets()
{
    charSets.clear();
    currCharSet = 0;
    update();
}

/*
  Display a character. \a speed determines how quickly the character is
  drawn.
*/
void QIMPenSettingsWidget::showCharacter( QIMPenChar *ch, int speed )
{
    outputChar = 0;
    outputStroke = 0;
    strokes.clear();
    mode = Output;
    repaint();
    if ( !ch || ch->isEmpty() ) {
        mode = Waiting;
        return;
    }

    outputChar = ch;
    outputStroke = outputChar->penStrokes().getFirst();
    if ( speed < 0 ) speed = 0;
    if ( speed > 20 ) speed = 20;
    speed = 50 - speed;
    pointIndex = 0;
    strokeIndex = 0;
    lastPoint = outputStroke->startingPoint();
    QRect br( outputChar->boundingRect() );
    lastPoint.setX( (width() - br.width()) / 2 + (lastPoint.x () - br.left()) );
    QPoint offset = lastPoint - outputStroke->startingPoint();
    br.moveBy( offset.x(), offset.y() );
    dirtyRect |= br;
    timer->start( speed );
}

/*!
  Handle drawing/clearing of characters.
*/
void QIMPenSettingsWidget::timeout()
{
    if ( mode == Output ) {
        const QArray<QIMPenGlyphLink> &chain = outputStroke->chain();
        if ( pointIndex < chain.count() ) {
            QPainter paint( this );
            paint.setBrush( Qt::black );
            for ( unsigned i = 0; i < 3 && pointIndex < chain.count(); i++ ) {
                lastPoint.rx() += chain[(int)pointIndex].dx;
                lastPoint.ry() += chain[(int)pointIndex].dy;
                pointIndex++;
                paint.drawRect( lastPoint.x()-1, lastPoint.y()-1, 2, 2 );
            }
        }
        if ( pointIndex >= chain.count() ) {
            QIMPenStrokeList strokes = outputChar->penStrokes();
            if ( strokeIndex < (int)strokes.count() - 1 ) {
                pointIndex = 0;
                strokeIndex++;
                outputStroke = strokes.at( strokeIndex );
                lastPoint = outputChar->startingPoint();
                QRect br( outputChar->boundingRect() );
                lastPoint.setX( (width() - br.width()) / 2
                                + (lastPoint.x () - br.left()) );
                QPoint off = lastPoint - outputChar->startingPoint();
                lastPoint = outputStroke->startingPoint() + off;
            } else {
                timer->stop();
                mode = Waiting;
            }
        }
    } else if ( mode == Waiting ) {
	QRect r( dirtyRect );
	if ( !r.isNull() ) {
	    r.moveBy( -2, -2 );
	    r.setSize( r.size() + QSize( 4, 4 ) );
	    repaint( r );
	}
    }
}

/*!
  If the point \a p is over one of the character set titles, switch
  to the set and return TRUE.
*/
bool QIMPenSettingsWidget::selectSet( QPoint p )
{
    if ( charSets.count() ) {
	CharSetEntryIterator it( charSets );
	int spos = 0;
	int idx = 0;
	for ( ; it.current(); ++it, idx++ ) {
	    int setWidth = width() * it.current()->stretch / totalStretch;
	    spos += setWidth;
	    if ( p.x() < spos ) {
		if ( idx != currCharSet ) {
		    currCharSet = idx;
		    update( 0, 0, width(), 12 );
		    emit changeCharSet( currCharSet );
		    emit changeCharSet( charSets.at(currCharSet)->cs );
		}
		break;
	    }
	}
    }

    return FALSE;
}

/*!
  Hopefully returns a sensible size.
*/
QSize QIMPenSettingsWidget::sizeHint()
{
    return QSize( TITLE_WIDTH * charSets.count(), 75 );
}

void QIMPenSettingsWidget::mousePressEvent( QMouseEvent *e )
{
    if ( !readOnly && e->button() == LeftButton && mode == Waiting ) {
        // if selectSet returns false the click was not over the
        // char set selectors.
        if ( !selectSet( e->pos() ) ) {
            // start of character input
            timer->stop();
            if ( outputChar ) {
                outputChar = 0;
                outputStroke = 0;
                repaint();
            }
            mode = Input;
            lastPoint = e->pos();
            emit beginStroke();
            inputStroke = new QIMPenStroke;
            strokes.append( inputStroke );
            inputStroke->beginInput( e->pos() );
            QPainter paint( this );
            paint.setBrush( Qt::black );
            paint.drawRect( lastPoint.x()-1, lastPoint.y()-1, 2, 2 );
        }
    }
}

void QIMPenSettingsWidget::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !readOnly && e->button() == LeftButton && mode == Input ) {
        mode = Waiting;
        inputStroke->endInput();
        if ( charSets.count() )
            emit stroke( inputStroke );
	inputStroke = 0;
    }
}

void QIMPenSettingsWidget::mouseMoveEvent( QMouseEvent *e )
{
    if ( !readOnly && mode == Input ) {
        int dx = QABS( e->pos().x() - lastPoint.x() );
        int dy = QABS( e->pos().y() - lastPoint.y() );
        if ( dx + dy > 1 ) {
            if ( inputStroke->addPoint( e->pos() ) ) {
		QPainter paint( this );
		paint.setPen( Qt::black );
		paint.setBrush( Qt::black );
		const QArray<QIMPenGlyphLink> &chain = inputStroke->chain();
		QPoint p( e->pos() );
		for ( int i = (int)chain.count()-1; i >= 0; i-- ) {
		    paint.drawRect( p.x()-1, p.y()-1, 2, 2 );
		    p.rx() -= chain[i].dx;
		    p.ry() -= chain[i].dy;
		    if ( p == lastPoint )
			break;
		}

		/* ### use this when thick lines work properly on all devices
		paint.setPen( QPen( Qt::black, 2 ) );
		paint.drawLine( lastPoint, e->pos() );
		*/
	    }
	    lastPoint = e->pos();
        }
    }
}

void QIMPenSettingsWidget::paintEvent( QPaintEvent * )
{
    QPainter paint( this );

    // draw guidelines
    paint.setPen( Qt::gray );
    paint.drawLine( 0, 0, width(), 0 );
    int y = height() / 3;
    paint.drawLine( 0, y, width(), y );
    y *= 2;
    paint.setPen( blue );
    paint.drawLine( 0, y, width(), y );
    paint.setPen( Qt::gray );

    if ( !charSets.count() )
	return;

    // draw the character set titles
    QFont selFont( "helvetica", 8, QFont::Bold ); // no tr
    QFont font( "helvetica", 8 ); // no tr
    CharSetEntryIterator it( charSets );
    int spos = 0;
    for ( ; it.current(); ++it ) {
	int setWidth = width() * it.current()->stretch / totalStretch;
        spos += setWidth;
	if ( it.current() != charSets.getLast() ) {
	    paint.drawLine( spos, 0, spos, 5 );
	    paint.drawLine( spos, height()-1, spos, height()-6 );
	}
	paint.setFont( font );
        /*
	int w = paint.fontMetrics().width( it.current()->cs->title() );
        int tpos = spos - setWidth / 2;
        //string is not translated
        paint.drawText( tpos - w/2, 0, w, 12, QPainter::AlignCenter,
                        it.current()->cs->title() );*/
    }

    // draw any character that should be displayed when repainted.
    QPoint off;
    const QIMPenStrokeList *stk = 0;
    if ( outputChar && mode == Waiting ) {
        stk = &outputChar->penStrokes();
        QPoint p( outputChar->startingPoint() );
        QRect br( outputChar->boundingRect() );
        p.setX( (width() - br.width()) / 2 + (p.x () - br.left()) );
        off = p - outputChar->startingPoint();
    } else if ( mode == Waiting ) {
        stk = &strokes;
	strokeColor = gray;
    }

    if ( stk && !stk->isEmpty() ) {
        paint.setPen( strokeColor );
        paint.setBrush( strokeColor );
        QIMPenStrokeIterator it( *stk );
        while ( it.current() ) {
            QPoint p = it.current()->startingPoint() + off;
            paint.drawRect( p.x()-1, p.y()-1, 2, 2 );
            const QArray<QIMPenGlyphLink> &chain = it.current()->chain();
            for ( int i = 0; i < (int)chain.count(); i++ ) {
                    p.rx() += chain[i].dx;
                    p.ry() += chain[i].dy;
                    paint.drawRect( p.x()-1, p.y()-1, 2, 2 );
            }
            ++it;
	    if ( it.atLast() && mode == Waiting )
		strokeColor = black;
        }
    }

    dirtyRect = QRect();

    // debug
/*
    if ( input ) {
        QArray<int> sig = input->sig();
        for ( unsigned i = 0; i < sig.count(); i++ ) {
            paint.drawPoint( 200 + i, height()/2 - sig[i] / 8 );
        }
    }
*/
}

void QIMPenSettingsWidget::resizeEvent( QResizeEvent *e )
{
    if ( mode == Output )
	showCharacter( outputChar, 0 );

    QWidget::resizeEvent( e );
}

