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
#include "pkim.h"
#include "charlist.h"
#include <qtopia/qdawg.h>
#include <qtopia/resource.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qtimer.h>
#include <qpixmapcache.h>
#include <qlayout.h>
#include <qfile.h>

/*! \class PkIM
  \brief The PkIM class provides an input method
  based on a standard telephone keypad.
*/


/*!
  \internal
  Constructs a PkIM object.
   */
PkIM::PkIM()
    : shift(FALSE), active(FALSE), lastUnicode(0), status(0), tip(0),
      tid_hold(0), tid_abcautoend(0), waitforrelease(FALSE),
      charList(0), microX(0), microY(0), useProbability(FALSE),
      actionsSinceChangeMode(0), mRestrictToHint(FALSE)
{
    Config globalcfg(Global::defaultButtonsFile(), Config::File); // No tr
    globalcfg.setGroup("ButtonTimings");
    abcAutoEndTime = globalcfg.readNumEntry("AutoAcceptTimeout", 1200);
    pressAndHoldTime = globalcfg.readNumEntry("ButtonHeldTimeout", 1500);

    matcherSet = new PkIMMatcherSet(this);
    connect(matcherSet, SIGNAL(modeChanged(PkIMMatcher*)),
	    this, SLOT(forwardModeChanged()));
}

/*!
  \internal
  Destroys the PkIM object.
  */
PkIM::~PkIM()
{
    if (symbolPicker)
	delete symbolPicker;
    if (wordPicker)
	delete wordPicker;
    if (modePicker)
	delete modePicker;
    delete charList;
    delete tip;
}

/*!
  \internal
  If any picker objects are visible passes micro focus to the visible
  picker object.
  */
void PkIM::setMicroFocus( int x, int y )
{
    microX = x;
    microY = y;
    if (active) {

	if (symbolPicker && symbolPicker->isVisible())
	    symbolPicker->setMicroFocus(x, y);
	if (wordPicker && wordPicker->isVisible())
	    wordPicker->setMicroFocus(x, y);
	if (modePicker && modePicker->isVisible())
	    modePicker->setMicroFocus(x, y);
    }
}

/*!
  \internal
  If \a isPress is true accepts the currently composed word.
  */
void PkIM::mouseHandler( int , int isPress)
{
    // parameters are/were index and isPress.
    //make click on word accept word? is most likely meaning.
    if (!isPress) {
	sendAndEnd();
    }
}

/*!
  \internal
  If \a s is empty turns the input method off.  Otherwise sets current input hint to \a s.
*/
void PkIM::setHint(const QString& s)
{
    QString h = s;
    if (h.right(4) == "only")
	h = h.left(h.length()-4);
    matcherSet->clearNamedMode();
    if (h.isEmpty()) {
	matcherSet->clearHintedMode();
	active = FALSE;
    } else {
	active = (matcherSet->setHintedMode(h) != 0);
    }
    if (active)
	setModePixmap();
    emit modeChanged();
}

/*!
  \internal
  Sets the current input mode to \a h.  if \a hasShift is true also
  sets the next character to be shifted to upper case.
*/
void PkIM::setMode(const QString &h, bool hasShift)
{
    if (h != matcherSet->currentMode()->id())
        sendAndEnd(); // actual mode change
    if (h != matcherSet->currentMode()->id() || hasShift != shift) {
	if (!h.isEmpty() && matcherSet->setCurrentMode(h)) {
	    active = TRUE;
	    shift = hasShift;
	    setModePixmap();
	    emit modeChanged();
	} else {
	    active = FALSE;
	    emit modeChanged();
	}
    }
}

/*!
  \internal
  Sends the set mode function to any cooperative plugins.
*/
void PkIM::forwardModeChanged()
{
    QString mode = matcherSet->currentMode()->id();
    mode += "pkim"; // no tr
    emit sendFunction("set mode " + mode); // no tr
    emit modeChanged();
    setModePixmap();
    lastUnicode = 0;
}

/*!
  \internal
  Adds the symbol \a unicode to the text and accepts it as input.
*/
void PkIM::symbolSelected(int unicode, int keycode)
{
    Q_UNUSED(keycode);
    text = QChar(unicode);
    sendAndEnd();
}

/*!
  \internal
  If \a s is a valid word choice, sets the text to that word and accpts the text as input.
*/
void PkIM::wordSelected(const QString &s)
{
    uint pos;
    for (pos = 0; pos < choices.count(); pos++) {
	if (choices[pos] == s)
	    break;
    }
    if (pos < choices.count()) {
	choice = pos;
	text = choices[choice];
	sendIMEvent( QWSServer::IMCompose, text, text.length(), 0 );

	sendAndEnd();
    }
}

/*!
  \internal
  Resets the input method to the start state.  Will accept any active compose text before resetting.
  */
void PkIM::reset()
{
    if (active) {
	sendAndEnd();
	active = TRUE;
	word = text = "";
	if (symbolPicker)
	    symbolPicker->hide();
	if (wordPicker)
	    wordPicker->hide();
	if (modePicker)
	    modePicker->hide();
	emit modeChanged();
    }
}

/*!
  \internal
  Adds the word \a word to the user words dictionary.
  */
void PkIM::addWordToDictionary(const QString& word)
{
    // If the user deletes a word, assume that they don't want it
    // to be automatically added back. They can always use the
    // tool that was used for deletion to add it manually.
    //
    if ( Global::dawg("deleted").contains(word) )
	return;

    QString lword = word.lower();

    if ( Global::dawg("preferred").contains(word)
      || Global::dawg("preferred").contains(lword) )
	return;
    if ( Global::addedDawg().contains(word)
      || Global::addedDawg().contains(lword) )
	return;
    if ( Global::fixedDawg().contains(word)
      || Global::fixedDawg().contains(lword) )
	return;

    Global::addWords(word);

    if ( !tip ) {
	tip = new QLabel("",0,0,WStyle_Customize | WStyle_Tool);
	tip_hider = new QTimer(this);
	tip->setBackgroundMode( QWidget::PaletteBase );
	connect(tip_hider,SIGNAL(timeout()),tip,SLOT(hide()));
    }
    tip->setText(tr("<small>Added</small>&nbsp;<b>%1</b>").arg(word)); // sep required, position required
    tip->resize(tip->sizeHint());
    tip->move(QMAX(0,microX-tip->width()),microY+2);
    tip->show();
    tip_hider->start(1000,TRUE);
}

/*!
  \internal
  Accepts the currently composed text
  */
void PkIM::sendAndEnd()
{
    sendIMEvent( QWSServer::IMEnd, text, text.length() );
    word += text;

    PkIMMatcher *matcher = matcherSet->currentMode();
    matcher->reset();
    choices.clear();

    int wl = word.length();
    if ( wl ) {
	QChar l = word[wl-1];
	if ( !l.isLetter() ) {
	    // Don't add in Dict mode, because most cases will be false
	    // positives (eg. user didn't look at screen), and the consequence
	    // is bad (garbage match next time).
	    if ( wl > 2 && !matcher->lookup() ) {
		word.truncate(wl-1);
		addWordToDictionary(word);
	    }
	    word = "";
	}
    }

    text = "";
    bool modech = shift;
    shift = FALSE;
    if (charList)
	charList->hide();
    if ( modech )
	setModePixmap();
}

struct WordWraps
{
    const char *prefix;
    const char *postfix;
};

WordWraps wraps[] = {
    { 0, "'s"},
    { "\"", "'s\""},
    { "\"", "\""},
    { "\"", "'s"},
    { "\"", 0},
    { 0, 0},
};

/*!
  \internal
  Regenerates the list of possible texts based on current keys pressed and sends the
  most likely one to the input widget as compose text.
  */
void PkIM::compose()
{
    bool added_number = FALSE;
    PkIMMatcher *matcher = matcherSet->currentMode();
    if ( matcher->count() > 0 )  {
	QString lastset = matcher->atReverse(0);
	// if best match for last char is punctuation
	// really should be 'lastset contains punc' 
	if (lastset[0].isPunct()) {
	    // base of current text, with each punc choice.
	    choices = matcher->findAll(text, QString::null);
	    choice = 0;
	    text = choices[choice];
	} else {
	    choices = matcher->choices();

	    if (matcher->count() == 1) {
		QStringList all = matcher->findAll();
		QStringList::Iterator it;
		for (it = all.begin(); it != all.end(); ++it)
		    if (!choices.contains(*it))
			choices.append(*it);
	    }

	    WordWraps *w = wraps;
	    while(choices.count() < 1 && (w->prefix != 0 || w->postfix != 0)) {
		choices = matcher->choices(TRUE, FALSE, w->prefix, w->postfix);
		w += 1;
	    }
	    // still no choices? try with number word
	    w = wraps;
	    while(choices.count() < 1 && (w->prefix != 0 || w->postfix != 0)) {
		QString nword = matcher->numberWord(w->prefix, w->postfix);
		if (!nword.isEmpty()) {
		    added_number = TRUE;
		    choices += nword;
		}
		w += 1;
	    }

	    // still no choices, then try anyting that may fit in prefix/postfix
	    w = wraps;
	    while(choices.count() < 1 && (w->prefix != 0 || w->postfix != 0)) {
		// only returns a word if sets have error values...
		QString wword = matcher->writtenWord(w->prefix, w->postfix);
		if (!wword.isEmpty())
		    choices += wword;
		w += 1;
	    }
	    // always append the number word as a choice.
	    if (!added_number) {
		QString nword = matcher->numberWord(QString::null, QString::null);
		if (!nword.isEmpty()) {
		    added_number = TRUE;
		    choices += nword;
		}
	    }
	    choice = 0;
	    if ( choices.count() >= 1 ) {
		text = choices[choice];
	    } else {
		int punc = text.length()-1;
		while ( punc>=0 && !text[punc].isPunct() )
		    punc--;
		// Punctuation in unfound word. Stop trying to compose.
		if ( punc >= 0 ) {
		    PkIMGuessList gl = matcher->pop();
		    sendIMEvent( QWSServer::IMCompose, text, text.length(), 0 );
		    sendAndEnd();
		    matcher->pushGuessSet(gl);
		    compose();
		    return;
		} else {
		    // try nword and written word.
		    QString nword = matcher->numberWord();
		    // only returns a word if sets have error values...
		    QString wword = matcher->writtenWord();
		    if (!nword.isEmpty())
			choices += nword;
		    if (!wword.isEmpty() && wword != nword)
			choices += wword;

		    // just grab anything we can.
		    if (choices.count() == 0) {
			QString prefix = text;
			while (choices.count() == 0) {
			    choices = matcher->findAll(prefix, QString::null);
			    if (prefix.isEmpty())
				break;
			    prefix = prefix.mid(1);
			}
		    }
		    text = choices[choice];
		}
	    }
	}
    } else {
	text = "";
    }
    sendIMEvent( QWSServer::IMCompose, text, text.length(), 0 );
}

/*!
  \internal
  Reverts the last set of possible characters sent to the object.

  \sa appendLookup()
*/
void PkIM::revertLookup()
{
    actionsSinceChangeMode++;
    PkIMMatcher *matcher = matcherSet->currentMode();
    if ( matcher->count() ) {
	emit revertGuess(); // for pen input method to get char matches right
	PkIMGuessList gl = matcher->pop(); 
	shift = gl.shift;
	text.truncate(matcher->count()-1);
	compose();
    }
    if ( !matcher->count() )
	sendAndEnd();
}

/*!
  \internal
  Appends the characters in \a set as a list of possible characters
  to follow the current text.

  For instance on a phone the 2 key might append the set 'abc'.
*/
void PkIM::appendLookup(const QString &set)
{
    actionsSinceChangeMode++;
    // add new set,
    matcherSet->currentMode()->pushSet(set, shift);
    if (shift) {
	shift = FALSE;
	setModePixmap();
    }
    compose();
}

/*!
  \internal
  Appends a guess generated by a cooperative plugin.
  Functions much like appendLookup(), but guesses can
  include error values and indicate characters span 
  multiple future guesses.  For instance,
  the t guess might span both a l and a _ guess.
*/
void PkIM::appendGuess(const IMIGuessList  &set)
{
    actionsSinceChangeMode++;

    PkIMGuessList gl = set;
    if (!matcherSet->currentMode()->lookup()) {
	if (gl.longest() != matcherSet->currentMode()->count()+1) {
	    // just push and compose
	    bool wasShift = shift;
	    sendAndEnd();
	    shift = wasShift;
	}
    }
    gl.shift = shift;
    matcherSet->currentMode()->pushGuessSet(gl);
    if (shift) {
	shift = FALSE;
	setModePixmap();
    }
    compose();
}

/*!
  \internal
  Updates the pixmap that indicates the current PkIM mode.
  */
void PkIM::setModePixmap()
{
    if ( status ) {
	QPixmap pm = matcherSet->currentMode()->pixmap(shift);
	status->setPixmap(pm);

	// poke layout
	//status->setIndent(1); // since sizehintwidth==0 screws up layout
	status->parentWidget()->parentWidget()->layout()->activate();
    }
}

/*!
  \internal
  Sets the status widget of PkIM to be \a widget and connects its clicked 
  signal to the mode popup slot.
*/
void PkIM::setStatusWidget(QToolButton *widget)
{
    if (status)
        disconnect(status, SIGNAL(clicked()), this, SLOT(modePopup()));

    status = widget;

    if (status)
        connect(status, SIGNAL(clicked()), this, SLOT(modePopup()));
}

