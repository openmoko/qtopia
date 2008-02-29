/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define QTOPIA_INTERNAL_LANGLIST
#include "inputmethods.h"

#include <qpe/config.h>
#include <qpe/qpeapplication.h>
#include <qpe/inputmethodinterface.h>
#include <qpe/qlibrary.h>
#include <qpe/global.h>

#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qwidgetstack.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qdir.h>
#include <stdlib.h>
#include <qtranslator.h>

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#include <qwsevent_qws.h>
#endif

#ifdef SINGLE_APP
#include "handwritingimpl.h"
#include "keyboardimpl.h"
#include "pickboardimpl.h"
#endif


/* XPM */
static const char * tri_xpm[]={
"9 9 2 1",
"a c #000000",
". c None",
".........",
".........",
".........",
"....a....",
"...aaa...",
"..aaaaa..",
".aaaaaaa.",
".........",
"........."};

static const int inputWidgetStyle = QWidget::WStyle_Customize | 
				    QWidget::WStyle_Tool |
				    QWidget::WStyle_StaysOnTop |
				    QWidget::WGroupLeader;

InputMethods::InputMethods( QWidget *parent ) :
    QWidget( parent, "InputMethods", WStyle_Tool | WStyle_Customize )
{
    mkeyboard = NULL;

    QHBoxLayout *hbox = new QHBoxLayout( this );

    kbdButton = new QToolButton( this );
    kbdButton->setFocusPolicy(NoFocus);
    kbdButton->setToggleButton( TRUE );
    kbdButton->setFixedHeight( parent->sizeHint().height() );
    kbdButton->setFixedWidth( 32 );
    kbdButton->setAutoRaise( TRUE );
    kbdButton->setUsesBigPixmap( TRUE );
    hbox->addWidget( kbdButton );
    connect( kbdButton, SIGNAL(toggled(bool)), this, SLOT(showKbd(bool)) );

    kbdChoice = new QToolButton( this );
    kbdChoice->setFocusPolicy(NoFocus);
    kbdChoice->setPixmap( QPixmap( (const char **)tri_xpm ) );
    kbdChoice->setFixedHeight( parent->sizeHint().height() );
    kbdChoice->setFixedWidth( 13 );
    kbdChoice->setAutoRaise( TRUE );
    hbox->addWidget( kbdChoice );
    connect( kbdChoice, SIGNAL(clicked()), this, SLOT(chooseKbd()) );

    connect( (QPEApplication*)qApp, SIGNAL(clientMoused()),
	    this, SLOT(resetStates()) );


    imButton = new QWidgetStack( this ); // later a widget stack
    imButton->setFocusPolicy(NoFocus);
    imButton->setFixedHeight( parent->sizeHint().height() );
    imButton->setFixedWidth( 32 );
    hbox->addWidget(imButton);

    imChoice = new QToolButton( this );
    imChoice->setFocusPolicy(NoFocus);
    imChoice->setPixmap( QPixmap( (const char **)tri_xpm ) );
    imChoice->setFixedHeight( parent->sizeHint().height() );
    imChoice->setFixedWidth( 13 );
    imChoice->setAutoRaise( TRUE );
    hbox->addWidget( imChoice );
    connect( imChoice, SIGNAL(clicked()), this, SLOT(chooseIm()) );

    loadInputMethods();
}

InputMethods::~InputMethods()
{
    unloadInputMethods();
}

void InputMethods::hideInputMethod()
{
    kbdButton->setOn( FALSE );
}

void InputMethods::showInputMethod()
{
    kbdButton->setOn( TRUE );
}

void InputMethods::showInputMethod(const QString& name)
{
    int i = 0;
    QValueList<InputMethod>::Iterator it;
    InputMethod *im = 0;
    for ( it = inputMethodList.begin(); it != inputMethodList.end(); ++it, i++ ) {
	if ( (*it).name() == name ) {
	    im = &(*it);
	    break;
	}
    }
    if ( im )
	chooseKeyboard(im);
}

void InputMethods::resetStates()
{
    if ( mkeyboard && !mkeyboard->newIM )
	mkeyboard->interface->resetState();
}

QRect InputMethods::inputRect() const
{
    if ( !mkeyboard || !mkeyboard->widget || !mkeyboard->widget->isVisible() )
	return QRect();
    else
	return mkeyboard->widget->geometry();
}

void InputMethods::unloadInputMethods()
{
#ifndef SINGLE_APP
    // reverse order of load
    for ( int i = inputMethodList.count()-1; i >= 0; i-- ) {
	InputMethod &im = inputMethodList[i];
	im.release();
	im.library->unload();
	delete im.library;
    }
    for ( int i = inputModifierList.count()-1; i >= 0; i-- ) {
	InputMethod &im = inputModifierList[i];
	im.release();
	im.library->unload();
	delete im.library;
    }
#endif
    inputMethodList.clear();
    inputModifierList.clear();
}

void InputMethods::loadInputMethods()
{
#ifndef SINGLE_APP
    hideInputMethod();
    mkeyboard = 0;

    unloadInputMethods();

    QString path = QPEApplication::qpeDir() + "/plugins/inputmethods";
    QDir dir( path, "lib*.so" );
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	InputMethodInterface *iface = 0;
	ExtInputMethodInterface *eface = 0;

	QLibrary *lib = new QLibrary( path + "/" + *it );

	qDebug("loading input method %s", (*it).latin1());
	if ( lib->queryInterface( IID_InputMethod, (QUnknownInterface**)&iface ) == QS_OK ) {
	    qDebug("old input method");
	    InputMethod input;
	    input.library = lib;
	    input.newIM = FALSE;
	    input.interface = iface;
	    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
	    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort,bool,bool)) );
	    inputMethodList.append( input );

	    QString type = (*it).left( (*it).find(".") );
	    QStringList langs = Global::languageList();
	    for (QStringList::ConstIterator lit = langs.begin(); lit!=langs.end(); ++lit) {
		QString lang = *lit;
		QTranslator * trans = new QTranslator(qApp);
		QString tfn = QPEApplication::qpeDir()+"/i18n/"+lang+"/"+type+".qm";
		if ( trans->load( tfn ))
		    qApp->installTranslator( trans );
		else
		    delete trans;
	    }

	} else if ( lib->queryInterface( IID_ExtInputMethod, (QUnknownInterface**)&eface ) == QS_OK ) {
	    qDebug("new input method");
	    InputMethod input;
	    input.library = lib;
	    input.newIM = TRUE;
	    input.extInterface = eface;
	    input.widget = input.extInterface->keyboardWidget( 0, inputWidgetStyle );
	    // may be either a simple, or advanced.
	    if (input.widget) {
		qDebug("its a keyboard");
		inputMethodList.append( input );
	    } else {
		qDebug("its a real im");
		input.widget = input.extInterface->statusWidget( 0, 0 );
		if (input.widget) { 
		    qDebug("blah");
		    inputModifierList.append( input );
		    imButton->addWidget(input.widget, inputModifierList.count());
		}
	    }

	} else {
	    qDebug("unkown");
	    delete lib;
	}
    }
#else
    InputMethod input;
    input.interface = new HandwritingImpl();
    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort,bool,bool)) );
    inputMethodList.append( input );
    input.interface = new KeyboardImpl();
    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort,bool,bool)) );
    inputMethodList.append( input );
    input.interface = new PickboardImpl();
    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort,bool,bool)) );
    inputMethodList.append( input );
#endif
    if ( !inputMethodList.isEmpty() ) {
	mkeyboard = &inputMethodList[0];
	kbdButton->setPixmap( *mkeyboard->icon() );
    }
    if ( !inputMethodList.isEmpty() )
	kbdButton->show();
    else
	kbdButton->hide();
    if ( inputMethodList.count() > 1 )
	kbdChoice->show();
    else
	kbdChoice->hide();


    QWSServer::setCurrentInputMethod( 0 );
    if ( !inputModifierList.isEmpty() ) {
	imethod = &inputModifierList[0];
	imButton->raiseWidget(imethod->widget);
	QWSServer::setCurrentInputMethod( imethod->extInterface->inputMethod() );
    }

    if ( !inputModifierList.isEmpty() )
	imButton->show();
    else
	imButton->hide();
    if ( inputModifierList.count() > 1 )
	imChoice->show();
    else
	imChoice->hide();
}

void InputMethods::chooseKbd()
{
    QPopupMenu pop( this );

    QString imname;
    if (imethod)
	imname = imethod->library->library().mid(imethod->library->library().findRev('/') + 1);

    int i = 0;
    int firstDepKbd = 0;

    QValueList<InputMethod>::Iterator it;
    for ( it = inputMethodList.begin(); it != inputMethodList.end(); ++it, i++ ) {
	// add empty new items, all old items.
	if (!(*it).newIM || (*it).extInterface->compatible().count() == 0 ) {
	    pop.insertItem( (*it).name(), i, firstDepKbd);
	    if ( mkeyboard == &(*it) )
		pop.setItemChecked( i, TRUE );

	    firstDepKbd++;
	} else if ( (*it).extInterface->compatible().contains(imname)) {
	    // check if we need to insert a sep.
	    if (firstDepKbd == i)
		pop.insertSeparator();
	    pop.insertItem( (*it).name(), i, -1);
	    if ( mkeyboard == &(*it) )
		pop.setItemChecked( i, TRUE );
	}
    }

    QPoint pt = mapToGlobal(kbdChoice->geometry().topRight());
    QSize s = pop.sizeHint();
    pt.ry() -= s.height();
    pt.rx() -= s.width();
    i = pop.exec( pt );
    if ( i == -1 )
	return;
    InputMethod *im = &inputMethodList[i];
    chooseKeyboard(im);
}

void InputMethods::chooseIm()
{
    QPopupMenu pop( this );

    int i = 0;
    QValueList<InputMethod>::Iterator it;
    for ( it = inputModifierList.begin(); it != inputModifierList.end(); ++it, i++ ) {
	pop.insertItem( (*it).name(), i );
	if ( imethod == &(*it) )
	    pop.setItemChecked( i, TRUE );
    }

    QPoint pt = mapToGlobal(kbdChoice->geometry().topRight());
    QSize s = pop.sizeHint();
    pt.ry() -= s.height();
    pt.rx() -= s.width();
    i = pop.exec( pt );
    if ( i == -1 )
	return;
    InputMethod *im = &inputModifierList[i];

    chooseMethod(im);
}

void InputMethods::chooseKeyboard(InputMethod* im)
{
    if ( im != mkeyboard ) {
	if ( mkeyboard && mkeyboard->widget->isVisible() )
	    mkeyboard->widget->hide();
	mkeyboard = im;
	kbdButton->setPixmap( *mkeyboard->icon() );
    }
    if ( !kbdButton->isOn() )
	kbdButton->setOn( TRUE );
    else
	showKbd( TRUE );
}


void InputMethods::chooseMethod(InputMethod* im)
{
    if ( im != imethod ) {
	QWSServer::setCurrentInputMethod( 0 );
	imethod = im;
	if ( imethod->newIM )
	    QWSServer::setCurrentInputMethod( imethod->extInterface->inputMethod() );
    }
}
void InputMethods::showKbd( bool on )
{
    if ( !mkeyboard )
	return;

    if ( on ) {
	mkeyboard->resetState();
	// HACK... Make the texteditor fit with all input methods
	// Input methods should also never use more than about 40% of the screen
	int height = QMIN( mkeyboard->widget->sizeHint().height(), 134 );
	mkeyboard->widget->resize( qApp->desktop()->width(), height );
	mkeyboard->widget->move( 0, mapToGlobal( QPoint() ).y() - height );
	mkeyboard->widget->show();
    } else {
	mkeyboard->widget->hide();
    }

    emit inputToggled( on );
}

bool InputMethods::shown() const
{
    return mkeyboard && mkeyboard->widget->isVisible();
}

QString InputMethods::currentShown() const
{
    return mkeyboard && mkeyboard->widget->isVisible()
	? mkeyboard->name() : QString::null;
}

void InputMethods::sendKey( ushort unicode, ushort scancode, ushort mod, bool press, bool repeat )
{
#if defined(Q_WS_QWS)
    QWSServer::sendKeyEvent( unicode, scancode, mod, press, repeat );
#endif
}
