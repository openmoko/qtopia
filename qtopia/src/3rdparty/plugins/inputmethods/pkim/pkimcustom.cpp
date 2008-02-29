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

#include "pkim.h"
#include "symbolpicker.h"
#include "charlist.h"
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>

//static const int abcAutoEndTime=700;
//static const int pressAndHoldTime=300;
static const int abcAutoEndTime=1200;
static const int pressAndHoldTime=1500;

/*!
  \internal
  Filters key events.  Returns True if the event is to be blocked, otherwise returns false.
  */
bool PkIM::filter(int unicode, int keycode, int modifiers, 
  bool isPress, bool autoRepeat)
{
    if ( waitforrelease ) {
	if ( isPress || autoRepeat )
	    return TRUE;
	waitforrelease = FALSE;
	return TRUE;
    }

    if (symbolPicker && symbolPicker->isVisible()) {
	if (!symbolPicker->filter(unicode, keycode, modifiers, isPress, autoRepeat))
	    symbolPicker->hide();
	return TRUE;
    }

    if (wordPicker && wordPicker->isVisible()) {
	if (!wordPicker->filter(unicode, keycode, modifiers, isPress, autoRepeat))
	    wordPicker->hide();
	return TRUE;
    }

    if (modePicker && modePicker->isVisible()) {
	if (!modePicker->filter(unicode, keycode, modifiers, isPress, autoRepeat))
	    modePicker->hide();
	return TRUE;
    }

    // doesn't filter anything if off
    if ( !active )
	return FALSE;
    if (autoRepeat && unicode)
	return TRUE;

    actionsSinceChangeMode++;
    // may change this to table lookup.
    PkIMMatcher *matcher = matcherSet->currentMode();
    QMap<QChar, PkIMChar> set = matcher->map();

    if (set.contains(unicode)) {
	setUseProbability(FALSE);
	PkIMChar item = set[unicode];
	if (isPress) {
	    //check if there is a hold for it.
	    if (item.holdfunc != noFunction) {
		tid_hold = startTimer(pressAndHoldTime);
		hold_uc = unicode;
		hold_key = keycode;
		hold_item = item;
		return TRUE; // we will hold, need to wait for release
	    }
	    // XXX Add check for single char taparg.
	    // nature of func may need to wait till we have spec that uses it though.
	    // in this case should just modify uni-code and pass through.
	    if (item.tapfunc != noFunction) {
		return TRUE; // we will do it on release.
	    }
	    // no tap set, no hold set.  send, end, and let this key go through.
	    sendAndEnd(); 
	    word = "";
	    return FALSE;
	} else {
	    // is release
	    if ( tid_hold ) {
		killTimer(tid_hold);
		tid_hold = 0;
	    }
	    // doesn't matter if there was a hold function, we are
	    // not doing it.
	    // check out tap functionalit.
	    // XXX Add check for single char taparg.
	    // this function different depending if tap or hold.
	    if (item.tapfunc == insertText) {
		if (!matcher->lookup()) {
		    if ( tid_abcautoend )
			killTimer(tid_abcautoend);
		    if (unicode == lastUnicode) {
			nextWord();
		    } else {
			bool wasShift = shift;
			sendAndEnd();
			shift = wasShift;

			appendLookup(item.taparg);

			if (choices.count() == 1) {
			    sendAndEnd();
			    tid_abcautoend = 0;
			}
		    }
		} else {
		    appendLookup(item.taparg);
		}
		if (!charList)
		    charList = new CharList();

		if (!matcher->lookup() && choices.count() > 1) {
		    if (item.showList) {
			charList->setChars(choices);
			if ( !charList->isVisible() )
			    charList->setMicroFocus(microX, microY);
			charList->setCurrent(choices[choice]);
			charList->show();
		    }
		    tid_abcautoend = startTimer(abcAutoEndTime);
		}
		lastUnicode = unicode;
		return TRUE;
	    }
	    if (item.tapfunc != noFunction) {
		applyFunction(item.tapfunc);
		return TRUE;
	    }
	    // no tap set, no tap function, if there is a hold though we might have delayed the press.
	    // if so, compose and send text now.
	    if (item.holdfunc != noFunction) {
		text += unicode;
		sendAndEnd();
		return TRUE;
	    }
	}
	return FALSE; // sendAndEnd done on press
    } else if ( isPress && (keycode == Key_BackSpace
#ifdef QTOPIA_PHONE
		|| (!Global::mousePreferred() && keycode == Key_Back)
#endif
		)) {
	if (text.length())  {
	    revertLookup();
	    return TRUE;
	} else if ( word.length() ) {
	    word.truncate(word.length()-1);
	}
	sendAndEnd();
    } else if (isPress && set.count() > 0) {
	/* if not in set, (and set not empty), and not any other special key (Back)
	   then send and end */
	word = "";
	sendAndEnd();
    }
    return FALSE;
}

/*!
  \internal
  The timeout for multi-tap input and held keys.
  Completes time based key behavior such as pressing
  and holde 2 to get a 2 character rather than 'abc' lookup
  set.
  */
void PkIM::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == tid_hold ) {
	killTimer(tid_hold);
	tid_hold = 0;
	PkIMChar item = hold_item;
	// insertText different for hold and tap
	if (item.holdfunc == insertText) {
	    // send the hold characer
	    text += item.holdarg;
	    sendAndEnd();
	    waitforrelease = TRUE;
	} else if (item.holdfunc != noFunction) {
	    applyFunction(item.holdfunc);
	    waitforrelease = TRUE;
	}
    } else if ( e->timerId() == tid_abcautoend ) {
	lastUnicode = 0;
	sendAndEnd();
	killTimer(tid_abcautoend);
	tid_abcautoend = 0;
    }
}

/*!
  \internal
  Applies the function specified
  for a key by the current mode.
  */
void PkIM::applyFunction(PkIMFunc fn)
{
    switch(fn) {
	case changeShift:
	    toggleShift();
	    break;
	case changeMode:
	    toggleMode();
	    break;
	case modifyText:
	    nextWord();
	    break;
	case insertSpace:
	    endWord();
	    break;
	case insertText:
	    // shouldn't get here
	    qWarning("insertText function not supposed to be handled in PkIM::applyFunction()");
	    break;
	case insertSymbol:
	    symbolPopup();
	    break;
	    // not implemented.
	case noFunction:
	    // do nothing.
	    ;
    }
}

// extensable version of above.
/*!
  \internal
  Applies a function sent by a cooperative input method.
  */
void PkIM::applyFunction(const QString &f)
{
    actionsSinceChangeMode++;
    if (f == "end word") { // no tr
	endWord(); // implies space.
    } else if (f == "change shift") { // no tr
	toggleShift();
    } else if (f == "change mode") { // no tr
	toggleMode();
    } else if (f == "select mode") { // no tr
	modePopup();
    } else if (f == "next word") { // no tr
	nextWord();
    } else if (f == "select word") { // no tr
	wordPopup();
    } else if (f == "insert symbol") { // no tr
	symbolPopup();
    }
}

/*!
  \internal
  Adds a space to the current compose text and accepts it as input.
*/
void PkIM::endWord()
{
    text += ' ';
    sendAndEnd();
}

/*!
  \internal
  Toggles whether the next character should be shifted to upper case or not.
  */
void PkIM::toggleShift()
{
    shift = !shift;
    setModePixmap();
}

/*!
  \internal
  Toggles between available modes.  If input has been made since
  the mode was toggled and is not currently the hinted mode for the
  input widget will change mode the hinted mode for the input widget.
  */
void PkIM::toggleMode()
{
    if(restrictToHint())
	return;
    sendAndEnd();
    // press, release.. here.
    if (actionsSinceChangeMode < 3) {
	matcherSet->nextMode();
    } else {
	matcherSet->toggleHinted();
    }
    setModePixmap();
    actionsSinceChangeMode = 0;
}

/*!
  \internal
  Sets the next word choice as the current compose text for the input widget.
  */
void PkIM::nextWord()
{
    if ( choices.count()) {
	choice = (choice+1)%choices.count();
	text = choices[choice];
	sendIMEvent( QWSServer::IMCompose, text, text.length(), 0 );
    }
}

/*!
  \internal
  Opens the symbol picker popup.
  */
void PkIM::symbolPopup()
{
    if(restrictToHint())
	return;
    if (!symbolPicker) {
	symbolPicker = new SymbolPicker();
	connect(symbolPicker, SIGNAL(symbolClicked(int, int)),
		this, SLOT(symbolSelected(int, int)));
    }
    sendAndEnd();
    symbolPicker->setAppFont(font());
    symbolPicker->setMicroFocus(microX, microY);
    symbolPicker->show();
}


/*!
  \internal
  Opens the word picker popup.
  */
void PkIM::wordPopup()
{
    if (choices.count()) {
	if (!wordPicker) {
	    wordPicker = new WordPicker();
	    connect(wordPicker, SIGNAL(wordChosen(const QString &)),
		    this, SLOT(wordSelected(const QString &)));
	}
	wordPicker->setChoices(choices);
	wordPicker->setAppFont(font());
	wordPicker->setMicroFocus(microX, microY);
	wordPicker->show();
    }
}


/*!
  \internal
  Opens the mode picker popup.
  */
void PkIM::modePopup()
{
    if (restrictToHint())
	return;
    if (!modePicker) {
	modePicker = new ModePicker(matcherSet);
	connect(modePicker, SIGNAL(modeSelected(const QString &, bool)),
		this, SLOT(setMode(const QString &, bool)));
    }
    modePicker->setAppFont(font());
    modePicker->setMicroFocus(microX, microY);
    modePicker->show();

}
