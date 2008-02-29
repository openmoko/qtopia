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
#ifndef KEYBOARDIMPL_H
#define KEYBOARDIMPL_H

#include <inputmethodinterface.h>
#include <keyboard.h>

class KeyboardFrame;

class KeyboardInputMethod : public QtopiaInputMethod
{
    Q_OBJECT
public:
    KeyboardInputMethod(QObject * =0);
    ~KeyboardInputMethod();

    QString name() const;
    QString identifier() const;
    QString version() const;

    State state() const;
    int properties() const { return RequireMouse | InputModifier | MenuItem; }
    QIcon icon() const;

    void setHint(const QString &, bool );

    // state managment.
    void reset();

    QWidget *inputWidget( QWidget *parent );
    QWSInputMethod *inputModifier( );

    void menuActionActivated(int v);
    QList<QIMActionDescription*> menuDescription();
private slots:
    void sendKeyPress( ushort unicode, ushort keycode, ushort modifiers, bool press, bool repeat );
private:
    Keyboard *input;
    QList<QIMActionDescription*> keyboardActionDescriptionList;
    void queryResponse ( int property, const QVariant & result );
};

#endif
