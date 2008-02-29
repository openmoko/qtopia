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

#include "match.h"

#include <qtopia/qdawg.h>
#include <qtopia/global.h>

#include <qapplication.h>
#include <qtimer.h>

#include <limits.h>

#define ERROR_THRESHOLD	    200000
#define LOOKAHEAD_ERROR	    2500
//#define DEBUG_QIMPEN

QIMPenMatch::QIMPenMatch( QObject *parent, const char *name )
    : QObject( parent, name ), errorThreshold(ERROR_THRESHOLD), badMatches(0), mCanvasHeight(0)
{
    strokes.setAutoDelete( TRUE );
    wordChars.setAutoDelete( TRUE );
    wordMatches.setAutoDelete( TRUE );

    multiTimer = new QTimer( this );
    connect( multiTimer, SIGNAL(timeout()), this, SLOT(endMulti()) );

    prevMatchChar = 0;
    prevMatchError = INT_MAX;
    charSet = 0;
    multiCharSet = 0;
    multiTimeout = 500;
    canErase = FALSE;
    doWordMatching = true;
}

QIMPenMatch::~QIMPenMatch()
{
}

void QIMPenMatch::setCharSet( QIMPenCharSet *cs )
{
    charSet = cs;
}

void QIMPenMatch::beginStroke()
{
    multiTimer->stop();
}

void QIMPenMatch::strokeEntered( QIMPenStroke *st )
{
#ifdef DEBUG_QIMPEN
    qDebug( "---------- new stroke -------------" );
#endif
    mCanvasHeight = 0; // reset
    strokes.append( new QIMPenStroke( *st ) );

    QIMPenChar testChar;
    QIMPenStrokeIterator it(strokes);
    for ( ; it.current(); ++it ) {
	testChar.addStroke( it.current() );
    }

    QIMPenCharMatchList ml;
    if ( strokes.count() > 1 && multiCharSet ) {
#ifdef DEBUG_QIMPEN
	qDebug( "Matching against multi set" );
#endif
	ml = multiCharSet->match( &testChar );
    } else {
#ifdef DEBUG_QIMPEN
	qDebug( "Matching against single set" );
#endif
	ml = charSet->match( &testChar );
    }

    processMatches( ml );
}

void QIMPenMatch::processMatches( QIMPenCharMatchList &ml )
{
#ifdef DEBUG_QIMPEN
    qDebug( "Entering strokes.count() = %d", strokes.count() );
#endif
    QIMPenCharMatch candidate1 = { INT_MAX, 0 };
    QIMPenCharMatch candidate2 = { INT_MAX, 0 };
    QIMPenCharMatchList ml2;

    if ( ml.count() ) {//&&
//	 ml.first().penChar->penStrokes().count() == strokes.count() ) {
	candidate1 = ml.first();
#ifdef DEBUG_QIMPEN
	qDebug( QString("Candidate1 = %1").arg(QChar(candidate1.penChar->character())) );
#endif
    }

    if ( strokes.count() > 1 ) {
	// See if the last stroke can match a new character
	QIMPenChar testChar;
	QIMPenStroke *st = strokes.at(strokes.count()-1);
	testChar.addStroke( st );
	ml2 = charSet->match( &testChar );
	if ( ml2.count() ) {
	    candidate2 = ml2.first();
#ifdef DEBUG_QIMPEN
	    qDebug( QString("Candidate2 = %1").arg(QChar(candidate2.penChar->character())) );
#endif
	}
    }

    bool eraseLast = FALSE;
    bool output = TRUE;

    if ( candidate1.penChar && candidate2.penChar ) {
	// Hmmm, a multi-stroke or a new character are both possible.
	// Bias the multi-stroke case.
	if ( QMAX(candidate2.error, prevMatchError)*3 < candidate1.error ) {
	    int i = strokes.count()-1;
	    while ( i-- ) {
		strokes.removeFirst();
		emit removeStroke();
	    }
	    prevMatchChar = candidate2.penChar;
	    prevMatchError = candidate2.error;
	    multiCharSet = charSet;
	    ml = ml2;
#ifdef DEBUG_QIMPEN
	    qDebug( "** Using Candidate2" );
#endif
	} else {
	    if ( (prevMatchChar->character() >> 16) != Qt::Key_Backspace &&
		 (prevMatchChar->character() >> 16) < QIMPenChar::ModeBase )
		eraseLast = TRUE;
	    prevMatchChar = candidate1.penChar;
	    prevMatchError = candidate1.error;
#ifdef DEBUG_QIMPEN
	    qDebug( "** Using Candidate1, with erase" );
#endif
	}
    } else if ( candidate1.penChar ) {
	if ( strokes.count() != 1 )
	    eraseLast = TRUE;
	else
	    multiCharSet = charSet;
	prevMatchChar = candidate1.penChar;
	prevMatchError = candidate1.error;
#ifdef DEBUG_QIMPEN
	qDebug( "** Using Candidate1" );
#endif
    } else if ( candidate2.penChar ) {
	int i = strokes.count()-1;
	while ( i-- ) {
	    strokes.removeFirst();
	    emit removeStroke();
	}
	prevMatchChar = candidate2.penChar;
	prevMatchError = candidate2.error;
	multiCharSet = charSet;
	ml = ml2;
#ifdef DEBUG_QIMPEN
	qDebug( "** Using Candidate2" );
#endif
    } else {
	if ( !ml.count() ) {
#ifdef DEBUG_QIMPEN
	    qDebug( "** Failed" );
#endif
	    canErase = FALSE;
	} else {
#ifdef DEBUG_QIMPEN
	    qDebug( "Need more strokes" );
#endif
	    if ( strokes.count() == 1 )
		canErase = FALSE;
	    multiCharSet = charSet;
	}
	output = FALSE;
	emit noMatch();
    }

    if ( eraseLast && canErase ) {
#ifdef DEBUG_QIMPEN
	qDebug( "deleting last" );
#endif
	emit erase();
	wordChars.removeLast();
	wordEntered.truncate( wordEntered.length() - 1 );
    }

    if ( output ) {
	updateLastCanvasHeight(ml);
	emit matchedCharacters( ml );
	uint code = prevMatchChar->character() >> 16;
	if ( code < QIMPenChar::ModeBase ) {
	    updateWordMatch( ml );
	    emit keypress( prevMatchChar->character() );
	}
	canErase = TRUE;
    }

    if ( strokes.count() )
	multiTimer->start( multiTimeout, TRUE );
}

void QIMPenMatch::updateLastCanvasHeight(const QIMPenCharMatchList &ml)
{
    /* work out actual canvas height, 3 numbers.
       1, bounds of matched char (in 75 height)
       2 bounds of actual stroke
       work out what the height of canvas would have
       been in same scale.
     */
    QListIterator<QIMPenStroke> it(strokes);
    int input_height = 0;
    for (; it.current(); ++it) {
	input_height = QMAX(it.current()->boundingRect().height(), input_height);
    }

    QIMPenChar *ch = ml.first().penChar;
    if (ch) {
	int matched_height = ch->boundingRect().height();
	
	// not done often, and need the precision.
	mCanvasHeight = int(float(input_height * 75) / float(matched_height));
    }
}

void QIMPenMatch::updateWordMatch( QIMPenCharMatchList &ml )
{
    if ( !ml.count() || !doWordMatching )
	return;
    int ch = ml.first().penChar->character();
    QChar qch( ch );
    int code = ch >> 16;
    if ( qch.isPunct() || qch.isSpace() ||
	 code == Qt::Key_Enter || code == Qt::Key_Return ||
	 code == Qt::Key_Tab || code == Qt::Key_Escape ) {
//	qDebug( "Word Matching: Clearing word" );
	wordChars.clear();
	wordMatches.clear();
	wordEntered = QString();
    } else if ( code == Qt::Key_Backspace ) {
	//qDebug( "Word Matching: Handle backspace" );
	wordChars.removeLast();
	wordEntered.truncate( wordEntered.length() - 1 );
	matchWords();
    } else {
	QIMPenChar *matchCh;

	wordChars.append( new QIMPenCharMatchList() );
	wordEntered += ml.first().penChar->character();

	QIMPenCharMatchList::Iterator it;
	for ( it = ml.begin(); it != ml.end(); ++it ) {
	    matchCh = (*it).penChar;

	    if ( matchCh->penStrokes().count() == strokes.count() ) {
		QChar ch(matchCh->character());
		if ( !ch.isPunct() && !ch.isSpace() ) {
		    wordChars.last()->append( QIMPenCharMatch( (*it) ) );
		}
	    }
	}
	matchWords();
    }
    if ( !wordMatches.count() || wordMatches.getFirst()->word != wordEntered )
	wordMatches.prepend( new MatchWord( wordEntered, 0 ) );
    emit matchedWords( wordMatches );
}

void QIMPenMatch::matchWords()
{
    if ( wordEntered.length() > 0 ) {
	// more leaniency if we don't have many matches
	if ( badMatches < 200 )
	    errorThreshold += (200 - badMatches) * 100;
    } else
	errorThreshold = ERROR_THRESHOLD;
    wordMatches.clear();
    goodMatches = 0;
    badMatches = 0;
    if ( wordChars.count() > 0 ) {
	maxGuess = (int)wordChars.count() * 2;
	if ( maxGuess < 3 )
	    maxGuess = 3;
	QString str;
	scanDict( Global::fixedDawg().root(), 0, str, 0 );
/*
	QListIterator<MatchWord> it( wordMatches);
	for ( ; it.current(); ++it ) {
	    qDebug( QString("Match word: %1").arg(it.current()->word) );
	}
*/
    }
    //qDebug( "Possibles: Good %d, total %d", goodMatches, wordMatches.count() );
    wordMatches.sort();
}

void QIMPenMatch::scanDict( const QDawg::Node* n, int ipos, const QString& str, int error )
{
    if ( !n )
	return;
    if ( error / (ipos+1) > errorThreshold )
	return;

    while (n) {
	if ( goodMatches > 20 )
	    break;
	if ( ipos < (int)wordChars.count() ) {
	    int i;
	    QChar testCh = QChar(n->letter());
	    QIMPenCharMatchList::Iterator it;
	    for ( i = 0, it = wordChars.at(ipos)->begin();
		  it != wordChars.at(ipos)->end() && i < 8; ++it, i++ ) {
		QChar ch( (*it).penChar->character() );
		if ( ch == testCh || ( !ipos && ch.lower() == testCh.lower() ) ) {
		    int newerr =  error + (*it).error;
		    if ( testCh.category() == QChar::Letter_Uppercase )
			ch = testCh;
		    QString newstr( str + ch );
		    if ( n->isWord() && ipos == (int)wordChars.count() - 1 ) {
			wordMatches.append( new MatchWord( newstr, newerr ) );
			goodMatches++;
		    }
		    scanDict( n->jump(), ipos+1, newstr, newerr );
		}
	    }
	} else if ( badMatches < 200 && ipos < maxGuess ) {
	    int d = ipos - wordChars.count();
	    int newerr = error + ERROR_THRESHOLD + LOOKAHEAD_ERROR*d;
	    QString newstr( str + n->letter() );
	    if ( n->isWord() ) {
		wordMatches.append( new MatchWord( newstr, newerr ) );
		badMatches++;
	    }
	    scanDict( n->jump(), ipos+1, newstr, newerr );
	}
	n = n->next();
    }
}

void QIMPenMatch::backspace()
{
    wordChars.removeLast();
    wordEntered.truncate( wordEntered.length() - 1 );
    matchWords();
    if ( !wordMatches.count() || wordMatches.getFirst()->word != wordEntered )
	wordMatches.prepend( new MatchWord( wordEntered, 0 ) );
    emit matchedWords( wordMatches );
    if ( wordEntered.length() )
	canErase = TRUE;
}

void QIMPenMatch::endMulti()
{
    int i = strokes.count();
    while ( i-- )
	emit removeStroke();
    strokes.clear();
    multiCharSet = 0;
}

void QIMPenMatch::resetState()
{
    if ( !wordEntered.isEmpty() ) {
	wordChars.clear();
	wordMatches.clear();
	wordEntered = QString();
	emit matchedWords( wordMatches );
	canErase = FALSE;
    }
}
