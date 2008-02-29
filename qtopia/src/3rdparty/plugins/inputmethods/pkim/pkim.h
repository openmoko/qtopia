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
/****************************************************************************
** $Id: .emacs,v 1.3 1998/02/20 15:06:53 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef PKIM_H
#define PKIM_H

#include "symbolpicker.h"
#include "wordpicker.h"
#include "modepicker.h"
#include "pkimmatcher.h"
#include <qtopia/qdawg.h>
#include <qwindowsystem_qws.h>
#include <qarray.h>
#include <qvaluestack.h>
#include <qguardedptr.h>

class QLabel;
class QPopupMenu;
class CharList;

class PkIM : public QWSInputMethod
{
    Q_OBJECT
public:
    PkIM();
    ~PkIM();

    void reset();
    void setMicroFocus( int x, int y );
    void mouseHandler( int index, int state );
    void setHint(const QString&);
    bool filter(int unicode, int keycode, int modifiers, 
			    bool isPress, bool autoRepeat);

    void setStatusWidget(QToolButton* l);

    void appendGuess(const IMIGuessList &);

    void applyFunction(const QString &);

    void setUseProbability(bool b) {
	useProbability = b;
    }

    bool isActive() const { return active; }

    bool restrictToHint() const { return mRestrictToHint; }
    void setRestrictToHint(bool b) { mRestrictToHint = b; }

signals:
    void sendFunction(const QString &);
    void modeChanged();
    void revertGuess();

private slots:
    void symbolSelected(int unicode, int keycode);
    void wordSelected(const QString &);

    void setMode(const QString &, bool);

    void forwardModeChanged();
    
    void modePopup();
    void applyFunction(PkIMFunc);
    void sendAndEnd();
    void endWord(); // implies space.
    void toggleShift();
    void toggleMode();
    void nextWord();
    void symbolPopup();
    void wordPopup();

private:

    void revertLookup();
    void appendLookup(const QString & set);
    void addWordToDictionary(const QString& word);

    bool shift;

    bool active;

    int lastUnicode;
    QString word; // typed word, potentially to be added to dict.
    QString text; // 'current text' buffer.
    QToolButton* status;
    QLabel* tip;
    QTimer* tip_hider;
    QStringList choices;
    int choice;
    int tid_hold;
    int tid_abcautoend;
    int hold_uc;
    int hold_key;
    PkIMChar hold_item;
    bool waitforrelease;
    QGuardedPtr<SymbolPicker> symbolPicker;
    QGuardedPtr<WordPicker> wordPicker;
    QGuardedPtr<ModePicker> modePicker;
    CharList *charList;
    int microX;
    int microY;

    bool useProbability;
    uint actionsSinceChangeMode;
    bool mRestrictToHint;
    int abcAutoEndTime;
    int pressAndHoldTime;

    PkIMMatcherSet *matcherSet;

    void setModePixmap();
    void timerEvent(QTimerEvent*);

    QStringList adjustChoices(const QStringList &) const;
    void compose();
};



#endif
