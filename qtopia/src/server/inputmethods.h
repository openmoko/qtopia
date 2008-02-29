/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __INPUT_METHODS_H__
#define __INPUT_METHODS_H__


#include <qwidget.h>
#include <qhbox.h>
#include <qvaluelist.h>
#include <qlist.h>
#include <qwindowsystem_qws.h>
#include <qtopia/private/inputmethodinterface_p.h>

class QToolButton;
class QWidgetStack;
class PluginLoader;

class InputMethodInterface;
class ExtInputMethodInterface;
class CoopInputMethodInterface;
class QUnknownInterface;

class InputMethod
{
public:
    InputMethod() : widget(0), statusWidget(0), interface(0) { }
    InputMethod(const InputMethod &o);
    QWidget *widget;
    QWidget *statusWidget;
    QString libName;
    union { 
	InputMethodInterface *interface;
	ExtInputMethodInterface *extInterface;
	CoopInputMethodInterface *coopInterface;
    };
    enum Style {
	Original,
	Extended,
	Cooperative,
    } style;

    QString name() const;
    QPixmap *icon() const;
    QUnknownInterface *iface();
    void resetState();

    int operator <(const InputMethod& o) const;
    int operator >(const InputMethod& o) const;
    int operator <=(const InputMethod& o) const;
    InputMethod &operator=(const InputMethod &o);

    bool compatible(const QString &);
    bool requirePen();
};

class InputMethodSelector : public QHBox
{
    Q_OBJECT
public:
    InputMethodSelector(QWidget *parent);
    ~InputMethodSelector();
    void add(const InputMethod &);
    void clear();

    InputMethod *current() const;

    void showInputMethod(bool);
    void setInputMethod(const QString &);

    void sort();
    uint count() const { return mCount; }

    // filters current list against libname l
    void filterAgainst(const QString &l);
public slots:
    // doesn't affect 'always on' IM's.
    void activateCurrent( bool );
signals:
    // some are auto activated, dep on type.
    void aboutToActivate();
    void activated(InputMethod *);

    void inputWidgetShown(bool);

private slots:
    void showList();

private:
    QWidgetStack *mButton;
    QToolButton *mChoice;

    InputMethod *mCurrent;
    QValueList<InputMethod> list;
    uint mCount;
    QString imname;
};

class InputMethods : public QWidget
{
    Q_OBJECT
public:
    enum IMType { Any=0, Mouse=1, Keypad=2 };

    InputMethods( QWidget *, IMType=Any );
    ~InputMethods();
    
    QRect inputRect() const;
    bool shown() const;
    QString currentShown() const; // name of interface
    void unloadInputMethods();
    void loadInputMethods();

public slots:
    void showInputMethod(const QString& id);
    void showInputMethod();
    void hideInputMethod();

signals:
    void inputToggled( bool on );

private slots:
    //void chooseKbd();
    //void chooseIm();

    void resetStates();

    void sendKey( ushort unicode, ushort scancode, ushort modifiers, bool, bool );
    void qcopReceive( const QCString &msg, const QByteArray &data );

    void choosePenBased(InputMethod *);
    void chooseKeyBased(InputMethod *);

    void addGuessToKey(const IMIGuessList &);
    void revertGuessToKey();
    void functionToKey(const QString &);

    void addGuessToPen(const IMIGuessList &);
    void revertGuessToPen();
    void functionToPen(const QString &);

    void updateIMVisibility();

    void updateHintMap(QWSWindow *, QWSServer::WindowEvent);

    void checkConnect();
    void checkDisconnect();
private:
    void chooseMethod(InputMethod* im);
    //void chooseKeyboard(InputMethod* im);
    void updateKeyboards(InputMethod *im);

    void checkGuessConnection(bool);
    void disconnectGuesses( CoopInputMethodInterface *penInterface, CoopInputMethodInterface *keyInterface );
    void connectGuesses( CoopInputMethodInterface *penInterface, CoopInputMethodInterface *keyInterface );
    void updateHint(int);

private:
    InputMethodSelector *penBased;
    InputMethodSelector *keyBased;
    // modify so chosen by how is used, not by plugin type.

    QValueList<QUnknownInterface*> ifaceList;
    PluginLoader *loader;
    IMType type;

    QWSInputMethod *currentIM; // ?? 
    QWSGestureMethod *currentGM; // ?? 
    bool guessesConnected;
    QMap<int, QString> hintMap;
    QMap<int, bool> restrictMap;
    int lastActiveWindow;
};


#endif // __INPUT_METHODS_H__

