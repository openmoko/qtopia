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

#ifndef __INPUT_METHODS_H__
#define __INPUT_METHODS_H__


#include <qpe/inputmethodinterface.h>

#include <qwidget.h>
#include <qvaluelist.h>

class QToolButton;
class QWidgetStack;
class QLibrary;

struct InputMethod
{
#ifndef QT_NO_COMPONENT
    QLibrary *library;
#endif
    QWidget *widget;
    bool newIM;
    union { 
	InputMethodInterface *interface;
	ExtInputMethodInterface *extInterface;
    };

    inline QString name() { return newIM ? extInterface->name() : interface->name(); } 
    inline QPixmap *icon() { return newIM ? extInterface->icon() : interface->icon(); } 
    inline void release() { if ( newIM ) extInterface->release(); else interface->release(); }
    inline void resetState() { if ( !newIM ) interface->resetState(); }
};

class InputMethods : public QWidget
{
    Q_OBJECT
public:
    InputMethods( QWidget * );
    ~InputMethods();
    
    QRect inputRect() const;
    bool shown() const;
    QString currentShown() const; // name of interface
    void showInputMethod(const QString& id);
    void showInputMethod();
    void hideInputMethod();
    void unloadInputMethods();
    void loadInputMethods();

signals:
    void inputToggled( bool on );

private slots:
    void chooseKbd();
    void chooseIm();
    void showKbd( bool );
    void resetStates();
    void sendKey( ushort unicode, ushort scancode, ushort modifiers, bool, bool );

private:
    void chooseMethod(InputMethod* im);
    void chooseKeyboard(InputMethod* im);
    QToolButton *kbdButton;
    QToolButton *kbdChoice;
    QWidgetStack *imButton; // later will be widget stack
    QToolButton *imChoice;
    InputMethod *mkeyboard;
    InputMethod *imethod;
    QValueList<InputMethod> inputMethodList;
    QValueList<InputMethod> inputModifierList;
};


#endif // __INPUT_METHODS_H__

