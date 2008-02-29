/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <QVariant>
#include <qwindowsystem_qws.h>

#include <qtopialog.h>
#include "dockedkeyboard.h"
#include "windowmanagement.h"

DockedKeyboard::DockedKeyboard(QWidget* parent, Qt::WFlags f) : Keyboard(parent, f)
{
    // The keyboard frame is meaningless after the DockedKeyboard IM is destroyed,
    // so keep control of it by never parenting DockedKeyboardFrame;
    // This should also help keep the keyboard on top of other widgets.
    Q_UNUSED(parent);
    Q_ASSERT(keyboardFrame);
    qLog(Input) << "DockedKeyboard Instatiated.";
};

void DockedKeyboard::queryResponse ( int , const QVariant & )
{
};


DockedKeyboard::~DockedKeyboard()
{
    qLog(Input) << "DockedKeyboard Destructing";
    if(keyboardFrame){
    }
};

QWidget* DockedKeyboard::frame(){
    return keyboardFrame;
};

void DockedKeyboard::resetState()
{
    qLog(Input) << "DockedKeyboard::resetState()";
    Keyboard::resetState();
};
