/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "match.h"

#include <qdawg.h>
#include <qtopianamespace.h>

#include <QtAlgorithms>

#include <qapplication.h>
#include <qtimer.h>

#include <limits.h>

#define ERROR_THRESHOLD     200000
#define LOOKAHEAD_ERROR     2500

QIMPenMatch::QIMPenMatch( QObject *parent, const char *name )
    : QObject( parent ), errorThreshold(ERROR_THRESHOLD), badMatches(0), mCanvasHeight(0)
{
    setObjectName( name );
    /*
    strokes.setAutoDelete( true );
    wordChars.setAutoDelete( true );
    wordMatches.setAutoDelete( true );
    */

    multiTimer = new QTimer( this );
    connect( multiTimer, SIGNAL(timeout()), this, SLOT(endMulti()) );

    prevMatchChar = 0;
    prevMatchError = INT_MAX;
    charSet = 0;
    multiCharSet = 0;
    multiTimeout = 500;
    canErase = false;
    doWordMatching = true;
}

QIMPenMatch::~QIMPenMatch()
{
    while ( strokes.count() ) delete strokes.takeLast();
    while ( wordChars.count() ) delete wordChars.takeLast();
    while ( wordMatches.count() ) delete wordMatches.takeLast();
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
    mCanvasHeight = 0; // reset
    strokes.append( new QIMPenStroke( *st ) );

    QIMPenChar testChar;
    foreach(QIMPenStroke *s, strokes) {
        testChar.addStroke( s );
    }

    QIMPenCharMatchList ml;
    if ( strokes.count() > 1 && multiCharSet ) {
        ml = multiCharSet->match( &testChar );
    } else {
        ml = charSet->match( &testChar );
    }

    processMatches( ml );
}

void QIMPenMatch::processMatches( QIMPenCharMatchList &ml )
{
    QIMPenCharMatch candidate1 = { INT_MAX, 0 };
    QIMPenCharMatch candidate2 = { INT_MAX, 0 };
    QIMPenCharMatchList ml2;

    if ( ml.count() ) {//&&
//       ml.first().penChar->penStrokes().count() == strokes.count() ) {
        candidate1 = ml.first();
    }

    if ( strokes.count() > 1 ) {
        // See if the last stroke can match a new character
        QIMPenChar testChar;
        QIMPenStroke *st = strokes.at(strokes.count()-1);
        testChar.addStroke( st );
        ml2 = charSet->match( &testChar );
        if ( ml2.count() ) {
            candidate2 = ml2.first();
        }
    }

    bool eraseLast = false;
    bool output = true;

    if ( candidate1.penChar && candidate2.penChar ) {
        // Hmmm, a multi-stroke or a new character are both possible.
        // Bias the multi-stroke case.
        if ( qMax(candidate2.error, prevMatchError)*3 < candidate1.error ) {
            int i = strokes.count()-1;
            while ( i-- ) {
                strokes.removeFirst();
                emit removeStroke();
            }
            prevMatchChar = candidate2.penChar;
            prevMatchError = candidate2.error;
            multiCharSet = charSet;
            ml = ml2;
        } else {
            //uint prevKey = prevMatchChar->key();
            if (!prevMatchChar->repCharacter().isNull())
                eraseLast = true;
            prevMatchChar = candidate1.penChar;
            prevMatchError = candidate1.error;
        }
    } else if ( candidate1.penChar ) {
        if ( strokes.count() != 1 )
            eraseLast = true;
        else
            multiCharSet = charSet;
        prevMatchChar = candidate1.penChar;
        prevMatchError = candidate1.error;
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
    } else {
        if ( !ml.count() ) {
            canErase = false;
        } else {
            if ( strokes.count() == 1 )
                canErase = false;
            multiCharSet = charSet;
        }
        output = false;
        emit noMatch();
    }

    if ( eraseLast && canErase ) {
        emit erase();
        if (!wordChars.isEmpty()) {
            wordChars.removeLast();
            wordEntered.truncate( wordEntered.length() - 1 );
        }
    }

    if ( output ) {
        updateLastCanvasHeight(ml);
        emit matchedCharacters( ml );
        uint key = prevMatchChar->key();
        QChar code = prevMatchChar->repCharacter();
        if ( !code.isNull() ) {
            updateWordMatch( ml );
            emit keypress( key, code );
        }
        canErase = true;
    }

    if ( strokes.count() )
        multiTimer->start( multiTimeout );
}

void QIMPenMatch::updateLastCanvasHeight(const QIMPenCharMatchList &ml)
{
    /* work out actual canvas height, 3 numbers.
       1, bounds of matched char (in 75 height)
       2 bounds of actual stroke
       work out what the height of canvas would have
       been in same scale.
     */
    int input_height = 0;
    foreach (QIMPenStroke *s, strokes) {
        input_height = qMax( s->boundingRect().height(), input_height);
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
    QChar qch = ml.first().penChar->repCharacter();
    int code = ml.first().penChar->key();
    if ( qch.isPunct() || qch.isSpace() ||
         code == Qt::Key_Enter || code == Qt::Key_Return ||
         code == Qt::Key_Tab || code == Qt::Key_Escape ) {
        wordChars.clear();
        wordMatches.clear();
        wordEntered = QString();
    } else if ( code == Qt::Key_Backspace ) {
        if (!wordChars.isEmpty()) {
            wordChars.removeLast();
            wordEntered.truncate( wordEntered.length() - 1 );
            matchWords();
        }
    } else {
        QIMPenChar *matchCh;

        wordChars.append( new QIMPenCharMatchList() );
        wordEntered += ml.first().penChar->repCharacter();

        foreach(QIMPenCharMatch match, ml) {
            matchCh = match.penChar;

            if ( matchCh->penStrokes().count() == strokes.count() ) {
                QChar ch(matchCh->repCharacter());
                if ( !ch.isPunct() && !ch.isSpace() ) {
                    wordChars.last()->append( QIMPenCharMatch( match ) );
                }
            }
        }
        matchWords();
    }
    if ( !wordMatches.count() || wordMatches[0]->word != wordEntered )
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
        scanDict( Qtopia::fixedDawg().root(), 0, str, 0 );
    }
    // TODO assume wordMatches.count is large (on average), otherwise
    // qBubbleSort might be better here = check with tests
    qStableSort( wordMatches.begin(), wordMatches.end() );
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
                QChar ch( (*it).penChar->repCharacter() );
                if ( ch == testCh || ( !ipos && ch.toLower() == testCh.toLower() ) ) {
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
    if (!wordChars.isEmpty()) {
        wordChars.removeLast();
        wordEntered.truncate( wordEntered.length() - 1 );
        matchWords();
        if ( !wordMatches.count() || wordMatches[0]->word != wordEntered )
            wordMatches.prepend( new MatchWord( wordEntered, 0 ) );
        emit matchedWords( wordMatches );
        if ( wordEntered.length() )
            canErase = true;
    }
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
        canErase = false;
    }
}
