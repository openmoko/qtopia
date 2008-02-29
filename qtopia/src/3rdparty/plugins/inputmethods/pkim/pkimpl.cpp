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
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qstringlist.h>
#include <qtoolbutton.h>
#include "pkimpl.h"
#include "pkim.h"


PkImpl::PkImpl()
    : input(0), icn(0), statWid(0), ref(0)
{
}

PkImpl::~PkImpl()
{
    delete input;
    delete icn;
    delete statWid;
}



QRESULT PkImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_CoopInputMethod )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( PkImpl )
}

//For a composing input method, the keyboardWidget() function returns 0:

QWidget *PkImpl::keyboardWidget( QWidget *, Qt::WFlags )
{
    return 0;
}


//The function Modifier input() returns the input method:

QWSInputMethod *PkImpl::inputModifier( )
{
    if ( !input )
	input = new PkIM( );
    return input;
}


//resetState() resets the state of the input method:

void PkImpl::resetState()
{
    if ( input )
	input->reset();
}

CoopInputMethodInterface::State PkImpl::state() const
{
    if (input && input->isActive()) 
	return CoopInputMethodInterface::Ready;
    else
	return CoopInputMethodInterface::Sleeping;
}

void PkImpl::connectStateChanged(QObject *o, const char *s)
{
    if ( !input )
	input = new PkIM( );
    QObject::connect(input, SIGNAL(modeChanged()), o, s);
}

void PkImpl::disconnectStateChanged(QObject *o, const char *s)
{
    if (input)
	QObject::disconnect(input, SIGNAL(modeChanged()), o, s);
}

void PkImpl::connectFunction(QObject *o, const char *s)
{
    if ( !input )
	input = new PkIM( );
    QObject::connect(input, SIGNAL(sendFunction(const QString &)), o, s);
}

void PkImpl::disconnectFunction(QObject *o, const char *s)
{
    if (input)
	QObject::disconnect(input, SIGNAL(sendFunction(const QString &)), o, s);
}

void PkImpl::connectRevertGuess(QObject *o, const char *s)
{
    if ( !input )
	input = new PkIM( );
    QObject::connect(input, SIGNAL(revertGuess()), o, s);
}

void PkImpl::disconnectRevertGuess(QObject *o, const char *s)
{
    if (input)
	QObject::disconnect(input, SIGNAL(revertGuess()), o, s);
}


//icon() returns the icon.

QPixmap *PkImpl::icon()
{
    if (!icn)
	icn = new QPixmap( Resource::loadPixmap("pkim/pkim") );
    return icn;
}


//name() returns the name:

QString PkImpl::name()
{
    return qApp->translate( "InputMethods", "Phone Keys" );
}

/*
For a composing input method, the widget returned by statusWidget()
will be placed in the taskbar when the input method is selected. This
widget is typically used to display status, and can also be used to
let the user interact with the input method.
*/

QWidget *PkImpl::statusWidget( QWidget *parent, Qt::WFlags )
{
    if (!statWid) {

	statWid = new QToolButton( parent );
	statWid->setFocusPolicy(QWidget::NoFocus);
	statWid->setToggleButton( FALSE );
	statWid->setAutoRaise( TRUE );
	//statWid->setUsesBigPixmap( TRUE );

	(void)inputModifier();
	input->setStatusWidget(statWid);
    }
    return statWid;
}



/*
The compatible() function can be used to say that this input method is
only compatible with certain other input methods. In this case, there
are no restrictions:
*/
QStringList PkImpl::compatible( )
{
    return QStringList();
}


/*
In qcopReceive(), we get notified when there is an event on the
inputmethod channel:
*/
void PkImpl::qcopReceive( const QCString &msg, const QByteArray &data )
{
    // XXX This could be the default implementation of this function.

    if ( msg == "inputMethodHint(int)" ) {
	QDataStream ds(data, IO_ReadOnly);
	int h;
	ds >> h;
	switch (h) {
	case (int)QPEApplication::Number:
	    input->setHint("int");
	    break;
	case (int)QPEApplication::PhoneNumber:
	    input->setHint("phone");
	    break;
	case (int)QPEApplication::Words:
	    input->setHint("words");
	    break;
	case (int)QPEApplication::Text:
	    input->setHint("text");
	    break;
	default:
	    input->setHint(QString::null);
	}
    } else if ( msg == "inputMethodHint(QString)" ) {
	QDataStream ds(data, IO_ReadOnly);
	QString h;
	ds >> h;
	input->setHint(h);
    }
}

int PkImpl::properties()
{
    return (RequireKeypad | KeyFilter | InterpretsGuess);
}

void PkImpl::appendGuess(const IMIGuessList & s)
{
    input->appendGuess(s);
}

/*
functions are
   change mode
   modify text
   end word
possible extra's are
   insert symbol
   change shift
but don't have to be obeyed.
 */
void PkImpl::function(const QString &f)
{
    input->applyFunction(f);
}

void PkImpl::setHint(const QString &hint)
{
    if (input)
	input->setHint(hint);
}

bool PkImpl::restrictToHint() const
{
    if ( !input )
	return FALSE;
    return input->restrictToHint();
}
void PkImpl::setRestrictToHint(bool b)
{
    if ( !input )
	input = new PkIM( );
    input->setRestrictToHint(b);
}



