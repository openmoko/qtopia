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

#define QTOPIA_INTERNAL_LANGLIST
#include "inputmethods.h"

#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/private/inputmethodinterface_p.h>
#include <qtopia/qlibrary.h>
#include <qtopia/global.h>
#include <qtopia/pluginloader.h>
#include <qtopia/resource.h>

#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qiconset.h>
#include <qtoolbutton.h>
#include <qwidgetstack.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qdir.h>
#include <stdlib.h>
#include <qtranslator.h>
#include <qtl.h>

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#include <qwsevent_qws.h>
#include <qcopchannel_qws.h>
#endif

#ifdef QT_NO_COMPONENT
#include "../plugins/inputmethods/handwriting/handwritingimpl.h"
#include "../plugins/inputmethods/keyboard/keyboardimpl.h"
#include "../3rdparty/plugins/inputmethods/pickboard/pickboardimpl.h"
#endif


static const int inputWidgetStyle = QWidget::WStyle_Customize | 
				    QWidget::WStyle_Tool |
				    QWidget::WStyle_StaysOnTop |
				    QWidget::WGroupLeader;


/*
  Slightly hacky: We use WStyle_Tool as a flag to say "this widget
  belongs to the IM system, so clicking it should not cause a reset".
 */
class IMToolButton : public QToolButton
{
public:
    IMToolButton::IMToolButton( QWidget *parent ) : QToolButton( parent )
    { setWFlags( WStyle_Tool ); }
    QSize sizeHint() const {
	if (pixmap() && ! pixmap()->isNull()) {
	    return pixmap()->size()+QSize(2,2);
	} else {
	    return QToolButton::sizeHint();
	}
    };
};

InputMethod::InputMethod(const InputMethod &o)
{
    widget = o.widget;
    statusWidget = o.statusWidget;
    libName = o.libName;
    interface = o.interface;
    style = o.style;
}

InputMethod &InputMethod::operator=(const InputMethod &o)
{
    widget = o.widget;
    statusWidget = o.statusWidget;
    libName = o.libName;
    interface = o.interface;
    style = o.style;

    return *this;
}

int InputMethod::operator <(const InputMethod& o) const
{
    return name() < o.name();
}
int InputMethod::operator >(const InputMethod& o) const
{
    return name() > o.name();
}
int InputMethod::operator <=(const InputMethod& o) const
{
    return name() <= o.name();
}


QString InputMethod::name() const
{
    switch (style)
    {
	case Original:
	    return interface->name();
	case Extended:
	    return extInterface->name();
	case Cooperative:
	    return coopInterface->name();
    }
    return QString::null;
}

QPixmap *InputMethod::icon() const
{
    switch (style)
    {
	case Original:
	    return interface->icon();
	case Extended:
	    return extInterface->icon();
	case Cooperative:
	    return coopInterface->icon();
    }
    return 0;
}

QUnknownInterface *InputMethod::iface()
{
    switch (style)
    {
	case Original:
	    return (QUnknownInterface *)interface;
	case Extended:
	    return (QUnknownInterface *)extInterface;
	case Cooperative:
	    return (QUnknownInterface *)coopInterface;
    }
    return 0;
}

void InputMethod::resetState()
{
    switch (style) {
	case Original:
	    interface->resetState();
	    break;
	case Extended:
	    extInterface->resetState();
	    break;
	case Cooperative:
	    coopInterface->resetState();
	    break;
    }
}

bool InputMethod::compatible(const QString &name)
{
    switch (style) {
	case Original:
	    return TRUE;
	case Extended:
	    if (name.isEmpty())
		return (extInterface->compatible().count() == 0);
	    else
		return extInterface->compatible().contains(name);
	case Cooperative:
	    if (name.isEmpty())
		return (coopInterface->compatible().count() == 0);
	    else
		return coopInterface->compatible().contains(name);
	default:
	    return FALSE;
    }
}

bool InputMethod::requirePen()
{
    switch (style) {
	default:
	case Original:
	    return TRUE;
	case Extended:
	    if (widget)
		return TRUE;
	    return FALSE;
	case Cooperative:
	    if (coopInterface->properties() & CoopInputMethodInterface::RequireMouse)
		return TRUE;
	    return FALSE;
    }
}


InputMethodSelector::InputMethodSelector(QWidget *parent)
    : QHBox(parent, 0), mCurrent(0), mCount(0)
{
    setBackgroundMode(PaletteButton);

    // some will want to open up the 'input' widget.
    // for that.. well we will see.
    mButton = new QWidgetStack(this);
    mButton->setBackgroundMode(PaletteButton);
    mButton->setFocusPolicy(NoFocus);

    mChoice = new QToolButton(this);
    mChoice->setBackgroundMode(PaletteButton);
    mChoice->setPixmap( Resource::loadPixmap("qpe/tri") );
    mChoice->setFixedWidth( 13 );
    mChoice->setAutoRaise( TRUE );
    connect( mChoice, SIGNAL(clicked()), this, SLOT(showList()) );
    mChoice->hide();// until more than one.
}

InputMethodSelector::~InputMethodSelector()
{
    // doesn't own anything other than children... do nothing.
}

// before, need to set libName, iface, style.
void InputMethodSelector::add(const InputMethod &im)
{
    InputMethod i = im;
    QWidget *status = 0;
    // use statusWidget, or make up button out of icon.
    // (if always on like pkim or full screen im, just a label.
    switch(i.style) {
	default:
	case InputMethod::Original:
	    break;
	case InputMethod::Extended:
	case InputMethod::Cooperative:
	    status = i.statusWidget;
	    break;
    }
    if (!status) {
	IMToolButton *iButton = new IMToolButton(mButton);
	iButton->setFocusPolicy(NoFocus);
	iButton->setToggleButton( TRUE );
	iButton->setAutoRaise( TRUE );
	iButton->setUsesBigPixmap( TRUE );
	iButton->setPixmap(*(i.icon()));
	status = iButton;
	connect(iButton, SIGNAL(toggled(bool)), this, SLOT(activateCurrent(bool)));
	i.statusWidget = iButton;
    }
    list.append(i);
    status->setBackgroundMode(PaletteButton);
    mButton->addWidget(status, mCount++);
    if (mCurrent == 0) {
	emit aboutToActivate();
	mCurrent = &list[list.count()-1];
	emit activated(mCurrent);
	mButton->raiseWidget(status);
    }
    if (mCount > 1)
	mChoice->show();
}

void InputMethodSelector::filterAgainst(const QString &n)
{
    imname = n;
    if ( mCurrent && !mCurrent->compatible(imname) )
	activateCurrent(FALSE);
}

void InputMethodSelector::showList()
{
    QPopupMenu pop( this );
    pop.setFocusPolicy( NoFocus ); //don't reset IM
   
   // don't have compatible list? need to add this later. 

    int i = 0;
    int firstDepKbd = 0;

    QValueList<InputMethod>::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it, i++ ) {
	// add empty new items, all old items.
	if ((*it).compatible(QString::null)) {
	    pop.insertItem( (*it).name(), i, firstDepKbd);
	    if ( mCurrent == &(*it) )
		pop.setItemChecked( i, TRUE );

	    firstDepKbd++;
	} else if ( (*it).compatible(imname)) {
	    // check if we need to insert a sep.
	    if (firstDepKbd == i)
		pop.insertSeparator();
	    pop.insertItem( (*it).name(), i, -1);
	    if ( mCurrent == &(*it) )
		pop.setItemChecked( i, TRUE );
	}
    }

    QPoint pt = mapToGlobal(mButton->geometry().topRight());
    QSize s = pop.sizeHint();
    pt.ry() -= s.height();
    pt.rx() -= s.width();
    i = pop.exec( pt );
    if ( i == -1 )
	return;
    if (mCurrent != &list[i]) {
	activateCurrent(FALSE);
	emit aboutToActivate();
	mCurrent = &list[i];
	emit activated(mCurrent);
	mButton->raiseWidget(i);
	activateCurrent(TRUE);
    }
}

void InputMethodSelector::activateCurrent( bool on )
{
    if (mCurrent && mCurrent->widget) {
	if ( on ) {
	    mCurrent->resetState();
	    // HACK... Make the texteditor fit with all input methods
	    // Input methods should also never use more than about 40% of the screen
	    int height = QMIN( mCurrent->widget->sizeHint().height(), 134 );
	    mCurrent->widget->resize( qApp->desktop()->width(), height );
	    if (mapToGlobal( QPoint() ).y() - height > 0)
		mCurrent->widget->move( 0, mapToGlobal( QPoint() ).y() - height );
	    else 
		mCurrent->widget->move( 0, mapToGlobal( QPoint() ).y() + mCurrent->statusWidget->height() );
	    if (mCurrent->statusWidget->inherits("QToolButton")) 
		((QToolButton*)mCurrent->statusWidget)->setOn(TRUE);
	    mCurrent->widget->show();
	} else {
	    if (mCurrent->statusWidget->inherits("QToolButton")) 
		((QToolButton*)mCurrent->statusWidget)->setOn(FALSE);
	    mCurrent->widget->hide();
	}
	// should be emitted if the screen is changing sizes.
	emit inputWidgetShown( on );
    }
}

void InputMethodSelector::clear()
{
    mChoice->hide();
    emit aboutToActivate();
    mCurrent = 0;
    emit activated(mCurrent);
    // could remove by id to.

    // also need to empty the stack.
    // for each interface, remove its widget.
    QValueList<InputMethod>::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	InputMethod im = *it;
	mButton->removeWidget(im.statusWidget);
    }
    list.clear();
    mCount = 0;
}

void InputMethodSelector::sort()
{
    if (list.count() > 1) {
	qHeapSort( list );
	QValueList<InputMethod>::Iterator it;
	for ( it = list.begin(); it != list.end(); ++it ) {
	    InputMethod im = *it;
	    mButton->removeWidget(im.statusWidget);
	}
	int i = 0;
	for ( it = list.begin(); it != list.end(); ++it, ++i ) {
	    InputMethod im = *it;
	    mButton->addWidget(im.statusWidget, i);
	}
	mButton->raiseWidget(mCurrent->statusWidget);
    }
}

InputMethod *InputMethodSelector::current() const
{
    return mCurrent;
}

void InputMethodSelector::setInputMethod(const QString &s)
{
    // set to current.
    QValueList<InputMethod>::Iterator it;
    int i = 0;
    for ( it = list.begin(); it != list.end(); ++it, ++i ) {
	InputMethod im = *it;
	if (im.name() == s) {
	    mButton->raiseWidget(i);
	    break;
	}
    }
}


InputMethods::InputMethods( QWidget *parent, IMType t ) :
    QWidget( parent, "InputMethods", WStyle_Tool | WStyle_Customize ),
    loader(0), type(t), currentIM(0), currentGM(0), guessesConnected(FALSE),
    lastActiveWindow(0)
{
    QHBoxLayout *hbox = new QHBoxLayout( this );

    penBased = new InputMethodSelector(this);
    connect(penBased, SIGNAL(activated(InputMethod *)),
	    this, SLOT(choosePenBased(InputMethod *)));
    connect(penBased, SIGNAL(aboutToActivate()),
	    this, SLOT(checkDisconnect()));
    connect(penBased, SIGNAL(inputWidgetShown(bool)),
	    this, SIGNAL(inputToggled(bool)));
    hbox->addWidget(penBased);

    keyBased = new InputMethodSelector(this);
    connect(keyBased, SIGNAL(activated(InputMethod *)),
	    this, SLOT(chooseKeyBased(InputMethod *)));
    connect(keyBased, SIGNAL(aboutToActivate()),
	    this, SLOT(checkDisconnect()));
    connect(keyBased, SIGNAL(inputWidgetShown(bool)),
	    this, SIGNAL(inputToggled(bool)));
    hbox->addWidget(keyBased);

    loadInputMethods();

    QCopChannel *channel = new QCopChannel( "QPE/IME", this );
    connect( channel, SIGNAL(received(const QCString&,const QByteArray&)),
	     this, SLOT(qcopReceive(const QCString&,const QByteArray&)) );
    connect( qwsServer, SIGNAL(windowEvent(QWSWindow *, QWSServer::WindowEvent)),
	    this, SLOT(updateHintMap(QWSWindow *, QWSServer::WindowEvent)));

    // might also add own win id since wouldn't have been added at start up.
}

InputMethods::~InputMethods()
{
    unloadInputMethods();
}

void InputMethods::hideInputMethod()
{
    penBased->activateCurrent(FALSE);
}

void InputMethods::showInputMethod()
{
    penBased->activateCurrent(TRUE);
}

void InputMethods::showInputMethod(const QString& name)
{
    penBased->setInputMethod(name);
    penBased->activateCurrent(TRUE);
}

void InputMethods::resetStates()
{
    // just the current ones.
    if (penBased->current())
	    penBased->current()->resetState();
    if (keyBased->current())
	    keyBased->current()->resetState();
}

QRect InputMethods::inputRect() const
{
    if (penBased->current() && penBased->current()->widget 
	    && penBased->current()->widget->isVisible())
	return penBased->current()->widget->geometry();
    return QRect();
}

void InputMethods::unloadInputMethods()
{
    checkGuessConnection(FALSE);
    if (currentIM) {
	QWSServer::setCurrentInputMethod( 0 );
	currentIM = 0;
    }
    if (currentGM) {
	QWSServer::setCurrentGestureMethod( 0 );
	currentGM = 0;
    }
    if ( loader ) {
	penBased->clear();
	keyBased->clear();
#ifndef QT_NO_COMPONENT
	int i;
	// reverse order of load
	for ( i = ifaceList.count()-1; i >= 0; i-- ) {
	    loader->releaseInterface( ifaceList[i] );
	}
	ifaceList.clear();
#endif
	delete loader;
	loader = 0;
    }
}

void InputMethods::loadInputMethods()
{
    keyBased->blockSignals(TRUE);
    penBased->blockSignals(TRUE);
#ifndef QT_NO_COMPONENT
    hideInputMethod();
    unloadInputMethods();

    loader = new PluginLoader( "inputmethods" );

    QStringList list = loader->list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	QUnknownInterface *face = 0;
	InputMethod input;
	input.libName = *it;
	// get the plugin.
	if ( loader->queryInterface( *it, IID_InputMethod, &face ) == QS_OK ) {
	    input.style = InputMethod::Original;
	    input.interface = (InputMethodInterface *)face;
	    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
	    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort,bool,bool)) );
	} else if ( loader->queryInterface( *it, IID_ExtInputMethod, &face ) == QS_OK ) {
	    input.extInterface = (ExtInputMethodInterface *)face;
	    input.style = InputMethod::Extended;
	    input.widget = input.extInterface->keyboardWidget( 0, inputWidgetStyle );
	    input.statusWidget = input.extInterface->statusWidget( 0, 0 );
	} else if ( loader->queryInterface( *it, IID_CoopInputMethod, &face ) == QS_OK ) {
	    input.coopInterface = (CoopInputMethodInterface *)face;
	    input.style = InputMethod::Cooperative;
	    input.widget = input.coopInterface->inputWidget( 0, 0 );
	    input.statusWidget = input.coopInterface->statusWidget( 0, 0 );
	} else {
	    face = 0;
	}
	// put plugin in right place.
	if (face) {
	    if (input.requirePen()) {
		if (type == Keypad) {
		    loader->releaseInterface(face);
		    face = 0;
		} else {
		    penBased->add(input);
		    ifaceList.append(face);
		}
	    } else {
		if (type == Mouse) {
		    loader->releaseInterface(face);
		    face = 0;
		} else {
		    keyBased->add(input);
		    ifaceList.append(face);
		}
	    }
	}
    }
    // TODO: should do sort on lists somehow.
    penBased->sort();
    keyBased->sort();
    //qHeapSort( inputModifierList );
#else
    InputMethod input;
    input.interface = new HandwritingImpl();
    input.style = InputMethod::Original;
    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort,bool,bool)) );
    penBased.add(input);
    input.interface = new KeyboardImpl();
    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort,bool,bool)) );
    penBased.add(input);
    input.interface = new PickboardImpl();
    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort,bool,bool)) );
    penBased.add(input);
#endif
    

    if ( keyBased->current()) {
	//keyBased->show?
	InputMethod *imethod = keyBased->current();
	if (imethod->style == InputMethod::Extended)
	    currentIM = imethod->extInterface->inputMethod();
	else
	    currentIM = imethod->coopInterface->inputModifier();
	QWSServer::setCurrentInputMethod( currentIM );

	// we need to update keyboards afterwards, as some of them
	// may not be compatible with the current input method
	updateKeyboards(imethod);
    } else {
	currentIM = 0;
    }


    if ( penBased->current() ) {
	InputMethod *imethod = penBased->current();
	if (imethod->style == InputMethod::Cooperative) {
	    currentGM = imethod->coopInterface->gestureModifier();
	    QWSServer::setCurrentGestureMethod( currentGM );
	}
    }

    checkGuessConnection(TRUE);
    updateIMVisibility();
    keyBased->blockSignals(FALSE);
    penBased->blockSignals(FALSE);
}

void InputMethods::checkConnect()
{
    checkGuessConnection(TRUE);
}

void InputMethods::checkDisconnect()
{
    checkGuessConnection(FALSE);
}

void InputMethods::checkGuessConnection(bool connect)
{
    if (penBased->current() || keyBased->current()) {
	InputMethod *pen = penBased->current();
	InputMethod *key = keyBased->current();
	bool penCoop = (pen && pen->style == InputMethod::Cooperative
		    && pen->coopInterface->properties() & CoopInputMethodInterface::ProvidesGuess);
	bool keyCoop = (key && key->style == InputMethod::Cooperative
		&& key->coopInterface->properties() & CoopInputMethodInterface::InterpretsGuess);
	if (penCoop || keyCoop)
	{
	    if (connect)
		connectGuesses(penCoop ? pen->coopInterface : 0, keyCoop ? key->coopInterface : 0);
	    else
		disconnectGuesses(penCoop ? pen->coopInterface : 0, keyCoop ? key->coopInterface : 0);
	}
    }
}

void InputMethods::connectGuesses(CoopInputMethodInterface *peni,
	CoopInputMethodInterface *keyi)
{
    if (!guessesConnected) {
	guessesConnected = TRUE;
	if (peni && keyi) {
	    peni->connectAppendGuess(this,
		    SLOT(addGuessToKey(const IMIGuessList &)));
	    peni->connectRevertGuess(this,
		    SLOT(revertGuessToKey()));
	    peni->connectFunction(this,
		    SLOT(functionToKey(const QString &)));

	    keyi->connectAppendGuess(this,
		    SLOT(addGuessToPen(const IMIGuessList &)));
	    keyi->connectRevertGuess(this,
		    SLOT(revertGuessToPen()));
	    keyi->connectFunction(this,
		    SLOT(functionToPen(const QString &)));
	}
	if (peni)
	    peni->connectStateChanged(this, SLOT(updateIMVisibility()));
	if (keyi)
	    keyi->connectStateChanged(this, SLOT(updateIMVisibility()));
    }
}

void InputMethods::disconnectGuesses(CoopInputMethodInterface *peni,
	CoopInputMethodInterface *keyi)
{
    if (guessesConnected) {
	guessesConnected = FALSE;
	if (peni && keyi) {
	    peni->disconnectAppendGuess(this,
		    SLOT(addGuessToKey(const IMIGuessList &)));
	    peni->disconnectRevertGuess(this,
		    SLOT(revertGuessToKey()));
	    peni->disconnectFunction(this,
		    SLOT(functionToPen(const QString &)));

	    keyi->disconnectAppendGuess(this,
		    SLOT(addGuessToPen(const IMIGuessList &)));
	    keyi->disconnectRevertGuess(this,
		    SLOT(revertGuessToPen()));
	    keyi->disconnectFunction(this,
		    SLOT(functionToPen(const QString &)));
	}
	if (peni)
	    peni->disconnectStateChanged(this, SLOT(updateIMVisibility()));
	if (keyi)
	    keyi->disconnectStateChanged(this, SLOT(updateIMVisibility()));
    }
}

void InputMethods::addGuessToKey(const IMIGuessList &list)
{
    if (keyBased->current()
	    && keyBased->current()->style == InputMethod::Cooperative)
	keyBased->current()->coopInterface->appendGuess(list);
}

void InputMethods::revertGuessToKey()
{
    //XXX
}

void InputMethods::functionToKey(const QString &s)
{
    if (keyBased->current()
	    && keyBased->current()->style == InputMethod::Cooperative)
	keyBased->current()->coopInterface->function(s);
}

void InputMethods::addGuessToPen(const IMIGuessList &list)
{
    if (penBased->current()
	    && penBased->current()->style == InputMethod::Cooperative)
	penBased->current()->coopInterface->appendGuess(list);
}

void InputMethods::revertGuessToPen()
{
    //XXX
}

void InputMethods::functionToPen(const QString &s)
{
    if (penBased->current()
	    && penBased->current()->style == InputMethod::Cooperative) {
	penBased->current()->coopInterface->function(s);
    }
}

void InputMethods::updateIMVisibility()
{
    // could use the int, but would need to check both types anyway.
    //keyBased, penBased
    if ( keyBased->count() == 0
	    ||
	    (keyBased->count() == 1 && keyBased->current()
	    && keyBased->current()->style == InputMethod::Cooperative
	    &&
	    (keyBased->current()->coopInterface->state() == CoopInputMethodInterface::Sleeping 
	     || keyBased->current()->coopInterface->restrictToHint() ) ) ) {
	keyBased->hide();
    } else {
	keyBased->show();
    }

    if ( penBased->count() == 0
	    ||
	    (penBased->count() == 1 && keyBased->current()
	    && penBased->current()->style == InputMethod::Cooperative
	    &&
	    (penBased->current()->coopInterface->state() == CoopInputMethodInterface::Sleeping 
	     || penBased->current()->coopInterface->restrictToHint() ) ) ) {
	penBased->hide();
#ifndef QTOPIA_PHONE
    } else {
	penBased->show();
#endif
    }
}

void InputMethods::choosePenBased(InputMethod* imethod)
{
    if (imethod && imethod->style == InputMethod::Cooperative) {
	currentGM = imethod->coopInterface->gestureModifier();
	QWSServer::setCurrentGestureMethod( currentGM );
    } else {
	QWSServer::setCurrentGestureMethod( 0 );
	currentGM = 0;
    }
    checkGuessConnection(TRUE);
}

// Updates the display of the soft keyboards available to the current input method
void InputMethods::updateKeyboards(InputMethod *im)
{
    if ( im ) {
	QString imname = im->libName.mid(im->libName.findRev('/') + 1);
	penBased->filterAgainst(imname);
    }
}

void InputMethods::chooseKeyBased(InputMethod* imethod)
{
    updateKeyboards( imethod );

    if ( imethod && imethod->style != InputMethod::Original ) {
	if (imethod->style == InputMethod::Extended)
	    currentIM = imethod->extInterface->inputMethod();
	else
	    currentIM = imethod->coopInterface->inputModifier();
	QWSServer::setCurrentInputMethod( currentIM );
    } else if (currentIM) {
	QWSServer::setCurrentInputMethod( 0 );
	currentIM = 0;
    }
    checkGuessConnection(TRUE);
}

void InputMethods::qcopReceive( const QCString &msg, const QByteArray &data )
{
    Q_INT32 wid = 0;
    if ( msg == "inputMethodHint(int,int)" ) {
	QDataStream ds(data, IO_ReadOnly);
	int h;
	ds >> h;
	ds >> wid;
	if (hintMap.contains(wid)) {
	    switch (h) {
		case (int)QPEApplication::Number:
		    hintMap[wid] = "int";
		    break;
		case (int)QPEApplication::PhoneNumber:
		    hintMap[wid] = "phone";
		    break;
		case (int)QPEApplication::Words:
		    hintMap[wid] = "words";
		    break;
		case (int)QPEApplication::Text:
		    hintMap[wid] = "text";
		    break;
		default:
		    hintMap[wid] = QString::null;
		    break;
	    }
	}
    } else if ( msg == "inputMethodHint(QString,int)" ) {
	QDataStream ds(data, IO_ReadOnly);
	QString h;
	ds >> h;
	ds >> wid;
	bool r;
	if (h.right(4) == "only") {
	    r = TRUE;
	} else {
	    r = FALSE;
	}
	if (hintMap.contains(wid))
	    hintMap[wid] = h;
	if (restrictMap.contains(wid))
	    restrictMap[wid] = r;
    }
    if (wid && wid == lastActiveWindow)
	updateHint(wid);
}

void InputMethods::updateHintMap(QWSWindow *w, QWSServer::WindowEvent e)
{
    if (!w)
	return;
    // one signal can be multiple events.
    if ((e & QWSServer::Create) == QWSServer::Create) {
	if (!hintMap.contains(w->winId()))
	    hintMap.insert(w->winId(), QString::null);
	if (!restrictMap.contains(w->winId()))
	    restrictMap.insert(w->winId(), FALSE);
    } else if ((e & QWSServer::Destroy) == QWSServer::Destroy) {
	if (hintMap.contains(w->winId()))
	    hintMap.remove(w->winId());
	if (restrictMap.contains(w->winId()))
	    restrictMap.remove(w->winId());
    }

    if ( (e & QWSServer::Active) == QWSServer::Active
	 && w->winId() != lastActiveWindow)  {
	lastActiveWindow = w->winId();
	updateHint(lastActiveWindow);
    }
}

/* TODO: Also... if hint null, don't just set the hint, remove the IM/GM
   Not a problem now, (well behaved plugins) but should be done for 
   misbehaved plugins.
 */
void InputMethods::updateHint(int wid)
{
    InputMethod *imethod = keyBased->current();
    if ( imethod ) {
	if ( imethod->style == InputMethod::Extended ) {
	    // fake a qcop
	    QByteArray fakeData;
	    QDataStream ds(fakeData, IO_WriteOnly);
	    ds << hintMap[wid];
	    imethod->extInterface->qcopReceive("inputMethodHint(QString)", fakeData);
	} else if (imethod->style == InputMethod::Cooperative ) {
	    imethod->coopInterface->setHint(hintMap[wid]);
	    imethod->coopInterface->setRestrictToHint(restrictMap[wid]);
	}
    }

    imethod = penBased->current();
    if ( imethod ) {
	if ( imethod->style == InputMethod::Extended ) {
	    // fake a qcop
	    QByteArray fakeData;
	    QDataStream ds(fakeData, IO_WriteOnly);
	    ds << hintMap[wid];
	    imethod->extInterface->qcopReceive("inputMethodHint(QString)", fakeData);
	} else if (imethod->style == InputMethod::Cooperative ) {
	    imethod->coopInterface->setHint(hintMap[wid]);
	    imethod->coopInterface->setRestrictToHint(restrictMap[wid]);
	}
    }
    updateIMVisibility();
}

bool InputMethods::shown() const
{
    return penBased->current() && penBased->current()->widget && penBased->current()->widget->isVisible();
}

QString InputMethods::currentShown() const
{
    return penBased->current() && penBased->current()->widget && penBased->current()->widget->isVisible()
	? penBased->current()->name() : QString::null;
}

void InputMethods::sendKey( ushort unicode, ushort scancode, ushort mod, bool press, bool repeat )
{
#if defined(Q_WS_QWS)
    QWSServer::sendKeyEvent( unicode, scancode, mod, press, repeat );
#endif
}
