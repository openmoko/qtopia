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

#ifndef _QIMPENMATCH_H_
#define _QIMPENMATCH_H_

#include "char.h"

#include <qtopia/qdawg.h>

#include <qlist.h>

class QTimer;
class QIMPenWidget;
class QIMPenSetup;
class QIMPenWordPick;

class QIMPenMatch : public QObject
{
    Q_OBJECT
public:
    QIMPenMatch( QObject *parent=0, const char *name=0 );
    virtual ~QIMPenMatch();

    void resetState();
    void backspace();
    void setMultiStrokeTimeout( int t ) { multiTimeout = t; }
    int multiStrokeTimeout( ) const { return multiTimeout; }

    const QString &word() const { return wordEntered; }

    void setWordMatchingEnabled( bool e ) { doWordMatching = e; }
    bool isWordMatchingEnabled() const { return doWordMatching; }

    // the adjusted canvas height of the last stroke
    int lastCanvasHeight() const { return mCanvasHeight; }

    struct MatchWord {
	MatchWord( const QString &w, int e ) { word = w; error = e; }
	QString word;
	int error;
    };

    class MatchWordList : public QList<MatchWord>
    {
    public:
	int compareItems( QCollection::Item item1, QCollection::Item item2 ) {
	    MatchWord *m1 = (MatchWord *)item1;
	    MatchWord *m2 = (MatchWord *)item2;
	    return m1->error - m2->error;
	}
    };

public slots:
    void setCharSet( QIMPenCharSet * );
    void beginStroke();
    void strokeEntered( QIMPenStroke *st );

signals:
    void erase();
    void noMatch();
    void removeStroke();
    void keypress( uint ch );
    void matchedCharacters( const QIMPenCharMatchList & );
    void matchedWords( const QIMPenMatch::MatchWordList & );

protected slots:
    void processMatches( QIMPenCharMatchList &ml );
    void endMulti();

protected:
    void updateWordMatch( QIMPenCharMatchList &ml );
    void matchWords();
    void scanDict( const QDawg::Node* n, int ipos, const QString& str, int error );

    void updateLastCanvasHeight( const QIMPenCharMatchList &ml );

private:
    QList<QIMPenStroke> strokes;
    QIMPenChar *prevMatchChar;
    int prevMatchError;
    QIMPenCharSet *charSet;
    QIMPenCharSet *multiCharSet;
    QList<QIMPenCharMatchList> wordChars;
    MatchWordList wordMatches;
    QString wordEntered;
    bool doWordMatching;
    bool canErase;
    int errorThreshold;
    int goodMatches;
    int badMatches;
    int maxGuess;
    QTimer *multiTimer;
    int multiTimeout;
    int mCanvasHeight;
};

#endif // _QIMPENINPUT_H_
