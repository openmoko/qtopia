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
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <qwindowsystem_qws.h>
#include "keyboardframe.h"
#include <qtopiaipcenvelope.h>
#include <QDebug> //tmp

/*
    Keyboard is an input method for Qtopia.  Keyboard displays a popup widget depiciting keys onscreen (KeyboardFrame), and converts them into key events.

    Currently, KeyboardFrame handles both the mouseEvents and creating the keyEvents, but it is anticipated that output will be re-routed through the Keyboard class in future, most likely in the form of IMEvents instead.

    \enum Keyboard::MenuActivationCode

    This enum describes the value for the different SoftMenu items that the Keyboard input method provides.

    \value RootItem The top "Keyboard" menu item
    \value SwapPosition The command to swap keyboard positions. 
                        No longer supported
    \value ShowKeyboard Shows the keyboard when it is hidden 
    \value HideKeyboard Hides the keyboard when it is visible
    \value DockKeyboard Docks the keyboard to the bottom of the screen,
            reducing the available area for other applications.
            Not yet supported.
    \value UnDockKeyboard Un-docks the keyboard to the bottom of the screen,
            increasing the available area for other applications.
            Not yet supported.

*/
class QAction;

class Keyboard : public QWSInputMethod
{
    Q_OBJECT
public:
    enum MenuActivationCode {
        Unknown = 0,
        RootItem = 1,
        SwapPosition = 2,
        ShowKeyboard,
        HideKeyboard,
        DockKeyboard,
        UnDockKeyboard
    };

    Keyboard( QWidget* parent=0, Qt::WFlags f=0 );
    virtual ~Keyboard();
    void queryResponse ( int property, const QVariant & result );

    QWidget* frame();
    void resetState();
    QAction* menuActionToDuplicate(){ return mAction;};
    void menuActionActivated(int v);

signals:
    void stateChanged();

public slots:
    void checkMicroFocus();
private slots:
    void swapPosition(bool);
protected:
    virtual void updateHandler(int type);
    KeyboardFrame *keyboardFrame;
    int microX;
    int microY;
    QAction* mAction;
};

#endif //KEYBOARD_H
