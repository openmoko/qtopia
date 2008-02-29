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
#ifndef KEYBOARDIMPL_H
#define KEYBOARDIMPL_H

#include <inputmethodinterface.h>
#include <QDebug>

class DockedKeyboard;
class KeyboardFrame;

class DockedKeyboardInputMethod : public QtopiaInputMethod
{
    Q_OBJECT
public:
    DockedKeyboardInputMethod(QObject * =0);
    ~DockedKeyboardInputMethod();

    QString name() const;
    QString identifier() const;
    QString version() const;

    void setHint(const QString& hint, bool);

    State state() const;
    int properties() const { return RequireMouse | InputModifier | InputWidget | DockedInputWidget; }
    QIcon icon() const;

    // state managment.
    void reset();

    QWidget *inputWidget( QWidget *parent );
    QWSInputMethod *inputModifier( );

private slots:
    void sendKeyPress( ushort unicode, ushort keycode, ushort modifiers, bool press, bool repeat );
private:
    DockedKeyboard *input;

    void queryResponse ( int property, const QVariant & result );
};

#endif
