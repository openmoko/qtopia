/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifndef DOCKEDKEYBOARD_H
#define DOCKEDKEYBOARD_H

#include <qwindowsystem_qws.h>
#include "../keyboard/keyboard.h"
#include <QDebug> //tmp

/*
    DockedKeyboard is an input method for Qtopia.  DockedKeyboard docks a popup widget depicting keys on the bottom of the screen and reduces the available screen for other widgets by this space.  The popup widget (KeyboardFrame), interprets mouse events and converts them into key events.

*/

//class QDockWidget;

class DockedKeyboard : public Keyboard
{
    Q_OBJECT
public:
    DockedKeyboard( QWidget* parent=0, Qt::WFlags f=0 );
    virtual ~DockedKeyboard();

    QWidget* frame();
    void resetState();
    void queryResponse ( int , const QVariant & );
private:
};

#endif //DOCKEDKEYBOARD_H
