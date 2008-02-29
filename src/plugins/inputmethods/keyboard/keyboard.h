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

#include <QDebug> //tmp

/*
    Keyboard is an input method for Qtopia.  Keyboard displays a popup widget depiciting keys onscreen (KeyboardFrame), and converts them into key events.

    Currently, KeyboardFrame handles both the mouseEvents and creating the keyEvents, but it is anticipated that output will be re-routed through the Keyboard class in future, most likely in the form of IMEvents instead.

*/
class QAction;

class Keyboard : public QWSInputMethod
{
    Q_OBJECT
public:
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
