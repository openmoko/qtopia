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

#ifndef _QIMPENMATCH_H_
#define _QIMPENMATCH_H_

#include <qtopia/mstroke/char.h>
#include <qlist.h>

class QTimer;
class QIMPenWidget;
class QIMPenSetup;

class QFSPenMatch : public QObject
{
    Q_OBJECT
public:
    struct Guess {
	Guess() : length(0), error(0), key(0) {}
	int length;
	uint error;
	uint key;
	QChar text;
    };

    QFSPenMatch( QObject *parent=0);
    virtual ~QFSPenMatch();

    void clear();
    void addStroke( QIMPenStroke * );
    QList<Guess> currentMatches() const { return mMatchList; }

    // the adjusted canvas height of the last stroke
    int lastCanvasHeight() const { return mCanvasHeight; }

    void setCharSet( QIMPenCharSet * );
    const QIMPenCharSet *charSet() const { return mCharSet; }

private:
    QList<QIMPenChar> mTestChars;

    QIMPenCharSet *mCharSet;
    QList<Guess> mMatchList;
    int mMatchCount;

    int mCanvasHeight;
};

#endif // _QIMPENINPUT_H_
