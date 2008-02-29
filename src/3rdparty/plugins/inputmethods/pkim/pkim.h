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

#include "modepicker.h"
#include <qtopia/inputmatch/symbolpicker.h>
#include <qtopia/inputmatch/wordpicker.h>
#include <qtopia/inputmatch/pkimmatcher.h>
#include <qdawg.h>
#include <qwindowsystem_qws.h>
#include <qtopia/mstroke/profile.h>

#include <QPointer>
#include "charmatch.h"

class QLabel;
class QPopupMenu;
class CharList;
class QIMPenStroke;
class QFSPenMatch;

class PkIM : public QWSInputMethod
{
    Q_OBJECT
public:
    PkIM();
    virtual ~PkIM();

    virtual bool filter(int unicode, int keycode, int modifiers,
            bool isPress, bool autoRepeat);
    virtual bool filter(const QPoint &pos, int state, int w);
    // used to determine state
    bool isFiltering() const { return cMode != Off; }
    
    void reset();
    void setMicroFocus( int x, int y );
    void mouseHandler( int index, int state );
    void setHint(const QString&);
    void setStatusWidget(QLabel* l) { status = l; }

    virtual void queryResponse( int property, const QVariant & result );

    bool isActive() const { return active; }

    bool restrictToHint() const { return mRestrictToHint; }
    void setRestrictToHint(bool b) { mRestrictToHint = b; }

    bool passwordHint() const;
    void setPasswordHint(bool b);

signals:
    void stateChanged(int);
protected slots:
    void sendMatchedCharacters(const QList<QFSPenMatch::Guess> &cl);
private slots:
    void symbolSelected(int unicode, int keycode);
    void wordSelected(const QString &);

    void setMode(const QString &, bool);
    void ignoreRestOfStroke();
    void pluginMessage(const QString &, const QByteArray &);
    void removeStroke();
//    void guessTimedOut();

protected:
    virtual void updateHandler ( int type );
    virtual void processAuxiliaryHints(QStringList auxiliaryHints);

private:
    QIMPenStroke *inputStroke;
    bool protectInputStroke;
    QTimer *lClickTimer;
    QTimer *lStrokeTimer;
    QTimer *lGuessTimer;
    bool ignoreNextMouse;
    bool midStroke;
    QFSPenMatch *penMatcher;
    InputMatcher *wordMatcher;
    int multiStrokeTimeout;
    bool shownHelp;
    bool aboutToChangeGuess;

    void revertLookup();
    void appendLookup(const QString & set);
    void addWordToDictionary(const QString& word);

    QIMPenProfile *profile;
    QList<QIMPenProfile *> profileList;

    enum Mode {
	Off,
	Phone,
	Int,
	Lower,
	Upper,
        Dict
    };
    Mode cMode;
    QPoint lastPoint;
    QIMPenCharSetList baseSets;
    int canvasHeight;
    int choicePos;
    QStringList choices;
    void loadProfiles();
    
    // Worker functions for interpreting input
    bool processInputMatcherChar(InputMatcher *matcher, int unicode, int keycode, bool isPress, bool autoRepeat);
    bool processBackspace();

    void selectProfile(const QString &);
    void appendGuess(const InputMatcherGuessList &);
    void appendGuess(const QList<QFSPenMatch::Guess> &);
    void updateWord();
    void updateChoices();

    void processStroke(QIMPenStroke *);
    bool shift;
    bool autoCapitalize;
    bool capitalizeNext;
    bool hintedAutoCapitalization;
    QString autoCapitalizingPunctuation;
    bool autoCapitalizeEveryWord;
#ifdef GREENPHONE_EFFECTS
    bool allowHandwriting;
#endif
    bool active;

    int lastUnicode;
    QString word; // typed word, potentially to be added to dict.
    QString text; // 'current text' buffer.
    QLabel* status;
    QLabel* tip;
    QTimer* tip_hider;
    int choice;
    int tid_abcautoend;
    int hold_uc;
    int hold_key;
    InputMatcherChar hold_item;
    bool waitforrelease;
    QPointer<SymbolPicker> symbolPicker;
    QPointer<WordPicker> wordPicker;
    QPointer<ModePicker> modePicker;
    CharList *charList;
    int microX;
    int microY;
    int strokeThreshold;

    uint actionsSinceChangeMode;
    bool mRestrictToHint;
    bool mPasswordHint;

    InputMatcherSet *matcherSet;

    void setModePixmap();
    void timerEvent(QTimerEvent*);

    void applyFunction(InputMatcherFunc);
    void sendAndEnd(bool forceCommit = false);

    void endWord(); // implies space.
    void toggleShift();
    void toggleMode();
    void nextWord();
    void symbolPopup();
    void wordPopup();
    void modePopup();

    QStringList adjustChoices(const QStringList &) const;
    void compose();

    enum holdstate {
        null = 0,
        waiting_for_hold,
        waiting_for_release,
    };
    holdstate key_hold_status;  
};



#endif
