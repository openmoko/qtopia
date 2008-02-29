/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

/*!
    QIMPenMatch interacts with QIMPenChar lists and the Qtopia dictionaries to turn lists of guesses about letters into lists of guesses about words.

    \sa QDawg, QIMPenChar
*/
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

/*!
    Destroy QIMPenMatch and clean up.
*/
QIMPenMatch::~QIMPenMatch()
{
    while ( strokes.count() ) delete strokes.takeLast();
    while ( wordChars.count() ) delete wordChars.takeLast();
    while ( wordMatches.count() ) delete wordMatches.takeLast();
}

/*!
    Sets the current QIMPenCharSet to \a cs.  This is the the set of characters and stroke signatures
*/
void QIMPenMatch::setCharSet( QIMPenCharSet *cs )
{
    charSet = cs;
}

/*!
    Begins the multi-stroke timer.
    \sa endMulti()
*/
void QIMPenMatch::beginStroke()
{
    multiTimer->stop();
}

// 
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

/*!
    Updates the word with the latest QIMPenCharMatchList (i.e. list of character guesses).  
    
    If the first guess for the new character is punctuation, whitespace, or any of Qt::Key_Enter, Qt::Key_Return, or Qt::Key_Escape then the current word in progress will be reset.
*/
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

/*!
    \internal
    Searches the fixedDawg for words that match the current list of character guesses
    \sa QIMPenCharMatchList
*/
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
        // scanDict outputs to wordMatches
        scanDict( Qtopia::fixedDawg().root(), 0, str, 0 );
    }
    // TODO assume wordMatches.count is large (on average), otherwise
    // qBubbleSort might be better here = check with tests
    qStableSort( wordMatches.begin(), wordMatches.end() );
}

/*!
    \internal

    This function is used to recursively scan QDawgs for matches to the current QIMPenCharMatchList (i.e. the current list of lists of letter-guesses).  It finds words at least as long as wordChars, and stores the words and associated errors it finds in wordMatches.

    This function is an implementation detail, and not part of the Qtopia API.

    \sa QDawg
*/
void QIMPenMatch::scanDict( const QDawg::Node* n, int ipos, const QString& str, int error )
{
    if ( !n )
        return;
    if ( error / (ipos+1) > errorThreshold )
        return;

    while (n) {
        if ( goodMatches > 20 ) // Magic number - never return more than 20 matches.
            break;
        if ( ipos < (int)wordChars.count() ) {
            int i;
            QChar testCh = QChar(n->letter());
            QIMPenCharMatchList::Iterator it;
            // Magic number - scan up to the first 8 character guesses at ipos.
            for ( i = 0, it = wordChars.at(ipos)->begin();
                  it != wordChars.at(ipos)->end() && i < 8; ++it, i++ ) {
                QChar ch( (*it).penChar->repCharacter() );
                if ( ch == testCh || ( !ipos && ch.toLower() == testCh.toLower() ) ) {
                    int newerr =  error + (*it).error;
                    if ( testCh.category() == QChar::Letter_Uppercase )
                        ch = testCh;
                    QString newstr( str + ch );
                    // Could potentially accept -2, -3 and -4 here as well and 
                    // check for common suffixes - s, ed, es, ing etc.
                    if ( n->isWord() && ipos == (int)wordChars.count() - 1 ) {
                        wordMatches.append( new MatchWord( newstr, newerr ) );
                        goodMatches++;
                    }
                    scanDict( n->jump(), ipos+1, newstr, newerr );
                }
            }
        // TEMP: Should this be, like, 2, and not 200? 
        // Would 1 be sufficient? Is this "Find me one potential longer word"?
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

/*!
    Deletes the last set of letter guesses (QIMPenCharMatchList) from the word currently being constructed.
*/
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

/*!
    \fn void QIMPenMatch::setMultiStrokeTimeout( int t )
*/

/*!
    \fn int QIMPenMatch::multiStrokeTimeout( ) const
*/

/*!
    \fn const QString &QIMPenMatch::word() const
*/

/*!
    \fn void QIMPenMatch::setWordMatchingEnabled( bool e )
    \sa isWordMatchingEnabled()
*/

/*!
    \fn bool QIMPenMatch::isWordMatchingEnabled() const
    Returns true if word matching (dictionary lookup) is currently enabled, otherwise returns false.
    \sa setWordMatchingEnabled()
*/

/*!
    \fn int QIMPenMatch::lastCanvasHeight() const 
    Returns the last height calculated for the canvas.  This value represents the height of the space that the last character was drawn on, calculated using the length of the stroke and the height of the character matched.

    If there has been no attept to match strokes, or the last such attempt failed, this value will be 0.
    \sa strokeEntered(), updateLastCanvasHeight()
*/

/*!
    This ends a multi-stroke, clearing internal data to make ready for a fresh stroke for a new letter.
    This function is called by a timer if a new stroke is not begun within a fixed time from the previous stroke being matched.
*/
void QIMPenMatch::endMulti()
{
    int i = strokes.count();
    while ( i-- )
        emit removeStroke();
    strokes.clear();
    multiCharSet = 0;
}

/*!
    This clears the internal data pertaining to words and emits the \l matchedWords signal with an empty \l MatchWordList.  \bold{Note:} This does not affect strokes, which will reset independantly.
    \sa endMulti(), backspace()
*/
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
