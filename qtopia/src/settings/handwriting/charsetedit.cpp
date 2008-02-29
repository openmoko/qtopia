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

#include "pensettingswidget.h"
#include "charsetedit.h"
#include "uniselect.h"

#include <qlistbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qbuttongroup.h>
#include <qslider.h>
#include <qtabwidget.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qaction.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#ifndef QTOPIA_PHONE
#include <qhbox.h>
#include <qtopia/qpemenubar.h>
#endif

#include <qtopia/contextmenu.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>

QIMPenInputCharDlg::QIMPenInputCharDlg( QWidget *parent, const char *name,
	bool modal, bool isFS, int WFlags)
    : QDialog( parent, name, modal, WFlags )
{
    setCaption( tr("New character") );
    uni = 0;

    QVBoxLayout *vb = new QVBoxLayout( this, 10 );
    vb->setMargin(4);
    vb->setSpacing(4);

    QHBoxLayout *hb = new QHBoxLayout();
    hb->setMargin(4);
    hb->setSpacing(4);

    QLabel *label = new QLabel( tr("Character"), this );
    hb->addWidget( label );

    QLineEdit *currentChar = new QLineEdit(this);
    currentChar->setAlignment(AlignHCenter);
    currentChar->setReadOnly(TRUE);
    hb->addWidget( currentChar );
    QPEApplication::setInputMethodHint(currentChar, QPEApplication::AlwaysOff);

    vb->addItem(hb);

#ifndef Q_WS_QWS
    hb = new QHBoxLayout();
    vb->addLayout( hb );

    QPushButton *pb = new QPushButton( tr("OK"), this );
    connect( pb, SIGNAL(clicked()), SLOT(accept()));
    hb->addWidget( pb );
    pb = new QPushButton( tr("Cancel"), this );
    connect( pb, SIGNAL(clicked()), SLOT(reject()));
    hb->addWidget( pb );
#endif

    u = new UniSelect(this);
    vb->addWidget(u);

    connect(u, SIGNAL(selected(const QString &)),
	    currentChar, SLOT(setText(const QString &)));
    connect(u, SIGNAL(selected(uint)),
	    this, SLOT(setCharacter(uint)));

#ifndef QTOPIA_PHONE
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    vb->addItem( spacer );
#endif

    u->setFocus();
    addSpecial( isFS );
    setCharacter(u->character());
    currentChar->setText(u->text());
}

// own map...
const int comboSel[] = {
    Qt::Key_Escape,
    Qt::Key_Tab,
    Qt::Key_Backspace,
    Qt::Key_Return,
    QIMPenChar::Caps,
    Qt::Key_unknown
};

const int popupComboSel[] = {
    QIMPenChar::CapsLock, // popup only
    QIMPenChar::Punctuation,
    QIMPenChar::Symbol, // popup only
    Qt::Key_unknown
};

const int fsComboSel[] = {
    //Qt::Key_Left,
    //Qt::Key_Right,
    //Qt::Key_Up,
    //Qt::Key_Down,
    QIMPenChar::Punctuation,
    QIMPenChar::NextWord, // fs only
    QIMPenChar::WordPopup, // fs only
    QIMPenChar::SymbolPopup, // fs only
    QIMPenChar::ModePopup, // fs only
    Qt::Key_unknown
};


void QIMPenInputCharDlg::addSpecial( bool isFS )
{
    int i = 0;
    while ( comboSel[i] != Key_unknown ) {
	QIMPenChar c;
	c.setCharacter(comboSel[i] << 16);
	u->addSpecial(c.character(), c.name());
	i++;
    }
    const int *extraSel;
    if (isFS)
	extraSel = fsComboSel;
    else
	extraSel = popupComboSel;
    i = 0;
    while ( extraSel[i] != Key_unknown ) {
	QIMPenChar c;
	c.setCharacter(extraSel[i] << 16);
	u->addSpecial(c.character(), c.name());
	i++;
    }
}

void QIMPenInputCharDlg::setCharacter( uint sp )
{
    uni = sp;
}

CharSetDlg::CharSetDlg( QWidget *parent, const char *name,
	    bool modal, int WFlags ): QDialog(parent, name, modal, WFlags)
{
    setCaption(tr("Handwriting"));
    QVBoxLayout *vl = new QVBoxLayout(this);

    edit = new CharSetEdit(this);

    vl->addWidget(edit);
}

CharSetDlg::~CharSetDlg() {}

void CharSetDlg::accept()
{
    edit->checkStoreMatch();
    QDialog::accept();
}

void CharSetDlg::reject()
{
    edit->checkStoreMatch();
    QDialog::reject();
}

void CharSetDlg::setCharSet( QIMPenCharSet *c )
{
    edit->setCharSet(c);
}

QIMPenCharSet *CharSetDlg::charSet() const
{
    return edit->charSet();
}


CharSetEdit::CharSetEdit( QWidget *parent, const char *name )
    : CharSetEditBase( parent, name ), currentSet(0),
	    lastCs(1), lastCh(0), addFlag(FALSE), mIsFS(FALSE)
{
    init();
}

CharSetEdit::CharSetEdit( QIMPenCharSet *c, QWidget *parent,
                const char *name )
    : CharSetEditBase( parent, name ), currentSet(0),
	    lastCs(1), lastCh(0), addFlag(FALSE)
{
    init();
    setCharSet(c);
}

void CharSetEdit::init()
{
    pw->setReadOnly(TRUE);
    currentChar = 0;
    currentCode = 0;
    matchCount = 0;
    matchIndex = 0;
    inputChar = new QIMPenChar();

    connect( newCharBtn, SIGNAL(clicked()), this, SLOT(addChar())) ;
    connect( delCharBtn, SIGNAL(clicked()), this, SLOT(removeChar())) ;
    connect( resetCharBtn, SIGNAL(clicked()), this, SLOT(resetMatches())) ;

    connect( charList, SIGNAL(highlighted(int)), SLOT(selectCode(int)) );

    pw->setFixedHeight( 75 );
    connect( pw, SIGNAL(stroke(QIMPenStroke*)),
                 SLOT(newStroke(QIMPenStroke*)) );

    connect( prevBtn, SIGNAL(clicked()), this, SLOT(prevMatch()));
    connect( nextBtn, SIGNAL(clicked()), this, SLOT(nextMatch()));
    connect( addBtn, SIGNAL(clicked()), this, SLOT(addMatch()) );
    connect( removeBtn, SIGNAL(clicked()), this, SLOT(removeMatch()) );

    charList->setFocus();

    enableButtons();
}

void CharSetEdit::setCharSet( QIMPenCharSet *c )
{
    if ( currentSet )
        pw->removeCharSet( 0 );
    currentSet = c;
    fillCharList();
    pw->insertCharSet( currentSet );
    inputChar->clear();
    if ( charList->count() ) {
        charList->setSelected( 0, TRUE );
        selectCode(0);
    }
}

QIMPenCharSet *CharSetEdit::charSet() const
{
    return currentSet;
}

class CharListItem : public QListBoxText
{
public:
    CharListItem( const QString &text, uint c )
	: QListBoxText( text ), _code(c) {}

    uint code() const { return _code; }

protected:
    uint _code;
};


/*!
  Fill the character list box with the characters.  Duplicates are not
  inserted.
*/
void CharSetEdit::fillCharList()
{
    charList->clear();
    QIMPenCharIterator it( currentSet->characters() );
    CharListItem *li = 0;
    for ( ; it.current(); ++it ) {
	uint ch = it.current()->character();
	QString n = it.current()->name();
	if ( !n.isEmpty() )
	    li = new CharListItem( n, ch );
	if ( li ) {
	    CharListItem *i = (CharListItem *)charList->findItem( li->text() );
	    if ( !i || i->code() != ch ) {
		charList->insertItem( li );
	    } else {
		delete li;
		li = 0;
	    }
	}
    }
    currentChar = 0;
}

void CharSetEdit::enableButtons()
{
}

/*!
  Find the previous character with the same code as the current one.
  returns 0 if there is no previous character.
*/
QIMPenChar *CharSetEdit::findPrev()
{
    if ( !currentChar )
	return 0;
    QIMPenCharIterator it( currentSet->characters() );
    bool found = FALSE;
    for ( it.toLast(); it.current(); --it ) {
        if ( !found && it.current() == currentChar )
            found = TRUE;
        else if ( found && it.current()->character() == currentCode &&
                !it.current()->testFlag( QIMPenChar::Deleted ) ) {
            return it.current();
        }
    }

    return 0;
}

/*!
  Find the next character with the same code as the current one.
  returns 0 if there is no next character.
*/
QIMPenChar *CharSetEdit::findNext()
{
    if ( !currentChar )
	return 0;
    QIMPenCharIterator it( currentSet->characters() );
    bool found = FALSE;
    for ( ; it.current(); ++it ) {
	if ( !found && it.current() == currentChar )
	    found = TRUE;
	else if ( found && it.current()->character() == currentCode &&
		    !it.current()->testFlag( QIMPenChar::Deleted ) ) {
	    return it.current();
	}
    }

    return 0;
}

void CharSetEdit::setCurrentChar( QIMPenChar *pc )
{
    currentChar = pc;
    pw->showCharacter( currentChar );
    if ( currentChar ) {
	if (currentChar->testFlag(QIMPenChar::System)) {
	    delCharBtn->setEnabled(FALSE);
	    resetCharBtn->setEnabled(FALSE);

	    bool haveMissing = FALSE;
	    QIMPenCharIterator it(currentSet->characters() );
	    for ( ; it.current(); ++it ) {
		if ( it.current()->character() == currentCode &&
			it.current()->testFlag( QIMPenChar::Deleted ) ) {
		    haveMissing = TRUE;
		    break;
		}
	    }
	    resetCharBtn->setEnabled(haveMissing);
	} else {
	    bool haveSystem = FALSE;
	    QIMPenCharIterator it(currentSet->characters() );
	    for ( ; it.current(); ++it ) {
		if ( it.current()->character() == currentCode &&
			it.current()->testFlag( QIMPenChar::System ) ) {
		    haveSystem = TRUE;
		    break;
		}
	    }
	    delCharBtn->setEnabled(!haveSystem);
	    resetCharBtn->setEnabled(haveSystem);
	}

    }
}

void CharSetEdit::prevMatch()
{
    // if not adding, or adding and something to add.
    if (!addFlag || !inputChar->isEmpty()) {
	if (addFlag) {
	    appendMatch();
	    pw->setReadOnly(TRUE);
	    addFlag = FALSE;
	}
	QIMPenChar *pc = findPrev();
	if ( pc ) {
	    setCurrentChar( pc );
	    --matchIndex;
	}
    } else if (addFlag) {
	// adding and something to add, (or would have met prev)
	matchCount--;
	matchIndex = matchCount;
	pw->showCharacter(currentChar);
	pw->setReadOnly(TRUE);
	addFlag = FALSE;
    }
    updateLabel();
}

void CharSetEdit::nextMatch()
{
    QIMPenChar *pc = findNext();
    if ( pc ) {
	setCurrentChar( pc );
	++matchIndex;
	updateLabel();
    }
}

void CharSetEdit::firstMatch()
{
    QIMPenCharIterator it( currentSet->characters() );
    for ( ; it.current(); ++it ) {
	if ( it.current()->character() == currentCode) {
	    if (it.current() != currentChar)
		setCurrentChar(it.current());
	    return;
	}
    }
}

void CharSetEdit::lastMatch()
{
    QIMPenCharIterator it( currentSet->characters() );
    QIMPenChar *lastFound = 0;
    for ( ; it.current(); ++it ) {
	if ( it.current()->character() == currentCode) {
	    lastFound = it.current();
	}
    }
    if (lastFound && lastFound != currentChar)
	setCurrentChar(lastFound);
}

void CharSetEdit::clearMatch()
{
    inputChar->clear();
    pw->clear();
    enableButtons();
}

void CharSetEdit::selectCode( int i )
{
    checkStoreMatch();
    currentChar = 0;
    currentCode = ((CharListItem *)charList->item(i))->code();
    QIMPenCharIterator it(currentSet->characters() );
    matchCount = 0;
    matchIndex = 0;
    for ( ; it.current(); ++it ) {
	if ( it.current()->character() == currentCode &&
	     !it.current()->testFlag( QIMPenChar::Deleted ) ) {
	    if (matchCount == 0) {
		setCurrentChar( it.current() );
		matchIndex = 1;
	    }
	    matchCount++;
	}
    }
    updateLabel();
    if ( matchCount == 0 )
	setCurrentChar( 0 );
    inputChar->clear();
    enableButtons();
    lastCh = i;
}

void CharSetEdit::updateLabel()
{
    QString itemText = tr("%1/%2", "way %1 of drawing character out of a total %2 ways of drawing character");
    itemDisplay->setText(itemText.arg(matchIndex).arg(matchCount));
    prevBtn->setEnabled( matchIndex > 1 );
    nextBtn->setEnabled( matchIndex < matchCount );

    removeBtn->setEnabled( matchCount > 0 );
}

/*
   Action should be:
   clearMatch,
   flag that what is drawn will be a new match
*/
void CharSetEdit::addMatch()
{
    checkStoreMatch();
    lastMatch();
    pw->setReadOnly(FALSE);
    addFlag = TRUE;
    matchCount++;
    matchIndex = matchCount;
    updateLabel();
    clearMatch();
    addBtn->setEnabled(FALSE);
}
    
void CharSetEdit::checkStoreMatch()
{
    if (addFlag) {
	addFlag = FALSE;
	appendMatch();
	pw->setReadOnly(TRUE);
    }
}

void CharSetEdit::appendMatch()
{
    // should be more of an assert.
    if ( !inputChar->isEmpty() ) {
        QIMPenChar *pc = new QIMPenChar( *inputChar );
        pc->setCharacter( currentCode );

	// User characters override all matching system characters.
	// Copy and mark deleted identical system characters.
	QIMPenCharIterator it(currentSet->characters() );
	QIMPenChar *sc = 0;
	while ( (sc = it.current()) != 0 ) {
	    ++it;
	    if ( sc->character() == currentCode &&
		 sc->testFlag( QIMPenChar::System ) &&
		 !sc->testFlag( QIMPenChar::Deleted ) )
	    {
		QIMPenChar *cc = new QIMPenChar( *sc );
		cc->clearFlag( QIMPenChar::System );
		currentSet->addChar( cc );
		sc->setFlag( QIMPenChar::Deleted );
	    }
	}

        currentSet->addChar( pc );
        setCurrentChar( pc );
        inputChar->clear();
	enableButtons();
    }
}

/* adds a whole new code */
void CharSetEdit::addChar()
{
    checkStoreMatch();
    //if ( !inputChar->isEmpty() ) {
    QIMPenInputCharDlg dlg( 0, "newchar", TRUE , mIsFS );
    if (QPEApplication::execDialog(&dlg)) {
	currentCode = dlg.unicode();
	// update combo now?  disable combo?
	// if added an existing char, do a new 'match'
	// if new code, then add code, set to current, add new match.
	bool foundMatch = FALSE;
	for (uint i = 0; i < charList->count(); ++i) {
	    if (((CharListItem *)charList->item(i))->code() == currentCode) {
		foundMatch = TRUE;
		charList->setCurrentItem(charList->item(i));
		break;
	    }
	}
	if (!foundMatch) {
	    // create a blank one.
	    QIMPenChar *pc = new QIMPenChar( *inputChar );
	    pc->setCharacter( currentCode );
	    CharListItem *cli = new CharListItem( pc->name(), currentCode);
	    charList->insertItem(cli);
	    charList->setCurrentItem(cli);
	}
	addMatch();
	updateLabel();
    }
}

/* removes a user added char */
void CharSetEdit::removeChar()
{
    addFlag = FALSE; // if was adding, just removed the char... can't add now
    pw->setReadOnly(TRUE);
    QIMPenCharIterator it(currentSet->characters() );
    it.toFirst();
    while ( it.current() ) {
	QIMPenChar *pc = it.current();
	++it;
	if ( pc->character() == currentCode ) {
	    if ( !pc->testFlag( QIMPenChar::System ) ) {
		currentSet->removeChar( pc );
	    }
	}
    }
    for (uint i = 0; i < charList->count();++i) {
	if (((CharListItem *)charList->item(i))->code() == currentCode) {
	    charList->removeItem(i);
	    break;
	}
    }
    updateLabel();
}

void CharSetEdit::removeMatch()
{
    if (addFlag) {
	// assume user meant cancel add match
	/// same as match prev when not added...
	//matchCount--;
	//matchIndex = matchCount;
	pw->showCharacter(currentChar);
	pw->setReadOnly(TRUE);
	addFlag = FALSE;
    } else {
	if ( currentChar ) {
	    QIMPenChar *pc = findPrev();
	    if ( !pc ) pc = findNext();
	    if ( currentChar->testFlag( QIMPenChar::System ) )
		currentChar->setFlag( QIMPenChar::Deleted );
	    else
		currentSet->removeChar( currentChar );
	    setCurrentChar( pc );
	}
    }
    if (matchIndex == matchCount)
	matchIndex--;
    matchCount--;
    updateLabel();
}

void CharSetEdit::resetMatches()
{
    if (addFlag) {
	addFlag = FALSE;
	pw->setReadOnly(TRUE);
    }
    if ( currentCode ) {
	currentChar = 0;
	bool haveSystem = FALSE;
	QIMPenCharIterator it(currentSet->characters() );
	for ( ; it.current(); ++it ) {
	    if ( it.current()->character() == currentCode &&
		 it.current()->testFlag( QIMPenChar::System ) ) {
		haveSystem = TRUE;
		break;
	    }
	}
	if ( haveSystem ) {
	    it.toFirst();
	    while ( it.current() ) {
		QIMPenChar *pc = it.current();
		++it;
		if ( pc->character() == currentCode ) {
		    if ( pc->testFlag( QIMPenChar::System ) ) {
			pc->clearFlag( QIMPenChar::Deleted );
			if ( !currentChar )
			    currentChar = pc;
		    } else {
			currentSet->removeChar( pc );
		    }
		}
	    }
	    setCurrentChar( currentChar );
	}
    }
    selectCode(charList->currentItem());
}

void CharSetEdit::newStroke( QIMPenStroke *st )
{
    inputChar->addStroke( st );

    addBtn->setEnabled(TRUE);
    if ( currentChar ) {
	bool haveSystem = FALSE;
	QIMPenCharIterator it(currentSet->characters() );
	for ( ; it.current(); ++it ) {
	    if ( it.current()->character() == currentCode &&
		    it.current()->testFlag( QIMPenChar::System ) ) {
		haveSystem = TRUE;
		break;
	    }
	}
	delCharBtn->setEnabled(!haveSystem);
	resetCharBtn->setEnabled(haveSystem);
    } else {
	// new Stroke on a new character....
	// can delete and delets troke and add match etc.
	delCharBtn->setEnabled(TRUE);
	resetCharBtn->setEnabled(FALSE);
    }
}

