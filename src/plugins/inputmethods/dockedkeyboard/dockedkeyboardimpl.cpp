/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include <QWSServer>
#include <qapplication.h>
#include <qpixmap.h>
#include "dockedkeyboard.h"
#include "dockedkeyboardimpl.h"
#include <qtopialog.h>

/* XPM */
static const char * kb_xpm[] = {
"28 13 4 1",
"       c None",
".      c #4C4C4C",
"+      c #FFF7DD",
"@      c #D6CFBA",
" .......................... ",
" .+++.+++.+++.+++.+++.++++. ",
" .+@@.+@@.+@@.+@@.+@@.+@@@. ",
" .......................... ",
" .+++++.+++.+++.+++.++++++. ",
" .+@@@@.+@@.+@@.+@@.+@@@@@. ",
" .......................... ",
" .++++++.+++.+++.+++.+++++. ",
" .+@@@@@.+@@.+@@.+@@.+@@@@. ",
" .......................... ",
" .++++.++++++++++++++.++++. ",
" .+@@@.+@@@@@@@@@@@@@.+@@@. ",
" .......................... "};

DockedKeyboardInputMethod::DockedKeyboardInputMethod(QObject *parent)
    : QtopiaInputMethod(parent), input(0)
{
}

DockedKeyboardInputMethod::~DockedKeyboardInputMethod()
{
    delete input;
}

QWidget *DockedKeyboardInputMethod::inputWidget( QWidget *parent )
{
    if ( !input ) {
        input = new DockedKeyboard( parent );
    }
    return input->frame();
}

void DockedKeyboardInputMethod::reset()
{
    if ( input )
        input->resetState();
}

QIcon DockedKeyboardInputMethod::icon() const
{
    QIcon i;
    i.addPixmap(QPixmap((const char **)kb_xpm));
    return i;
}

QString DockedKeyboardInputMethod::name() const
{
    return qApp->translate( "InputMethods", "DockedKeyboard" );
}

QString DockedKeyboardInputMethod::identifier() const
{
    return "http://trolltech.com/Qtopia/DockedKeyboardInputMethod";
}

QString DockedKeyboardInputMethod::version() const
{
    return "4.0.0";
}

QtopiaInputMethod::State DockedKeyboardInputMethod::state() const
{
    if (input && input->frame()->isVisible())
        return Ready;
  else
    return Sleeping;
}
void DockedKeyboardInputMethod::sendKeyPress(ushort k, ushort u, ushort, bool p, bool a)
{
    QWSServer::sendKeyEvent(k, u, 0, p, a);
}

QWSInputMethod *DockedKeyboardInputMethod::inputModifier( )
{
    if ( !input ) {
        qLog(Input) << "DockedKeyboardInPutMethod::inputModifier";
        input = new DockedKeyboard();
        connect (input, SIGNAL(stateChanged()), this, SIGNAL(stateChanged()));
    }
    return input;
}

void DockedKeyboardInputMethod::setHint(const QString &hint, bool)
{
    if(input && hint.isEmpty() && input->frame())
    {
        input->resetState();
    }
}
QTOPIA_EXPORT_PLUGIN(DockedKeyboardInputMethod);
