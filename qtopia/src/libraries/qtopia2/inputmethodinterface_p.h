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

#ifndef INPUTMETHODINTERFACE_P_H
#define INPUTMETHODINTERFACE_P_H

#include <qtopia/inputmethodinterface.h>

// {b18b0cc2-7db9-48ae-9939-f40cc5f95e93}
#ifndef IID_CoopInputMethod
#define IID_CoopInputMethod QUuid( 0xb18b0cc2, 0x7db9, 0x48ae, 0x99, 0x39, 0xf4, 0x0c, 0xc5, 0xf9, 0x5e, 0x93)
#endif

#include <qvaluelist.h>
struct IMIGuess {
    IMIGuess() : c(0x0000ffff), length(0), error(0) {}
    uint c;
    int length;
    uint error;
};

class IMIGuessList : public QValueList<IMIGuess>
{
public:
    IMIGuessList() : QValueList<IMIGuess>() {}
    IMIGuessList(const QValueList<IMIGuess> &o) : QValueList<IMIGuess>(o) {}
    IMIGuessList(const IMIGuessList &o) : QValueList<IMIGuess>(o) {}

    IMIGuessList &operator=(const QValueList<IMIGuess> &o)
    {
	QValueList<IMIGuess>::operator=(o);
	return *this;
    }
};

/* TODO requires good documentation */
struct CoopInputMethodInterface : public QUnknownInterface
{
    enum Properties {
	RequireMouse = 0x0001,
	RequireKeypad = 0x0002,
	MouseFilter = 0x0004,
	KeyFilter = 0x0008,
	ProvidesGuess = 0x0010,
	InterpretsGuess = 0x0020
    };

    //identifying functions.
    virtual QString name() = 0;
    virtual QPixmap *icon() = 0;
    
    // a small widget that shows current status.  can be interacted with,
    // but MUST be small.
    // return 0 if should just use icon.
    virtual QWidget *statusWidget( QWidget *parent, Qt::WFlags f )= 0;


    enum State {
	Sleeping = 1, // not producing input
	Ready = 2, // active/producing input
    };

    virtual State state() const = 0;
    virtual void connectStateChanged(QObject *r, const char *s) = 0;
    virtual void disconnectStateChanged(QObject *r, const char *s) = 0;

    // state managment.
    virtual void resetState() = 0;

    // proprities
    virtual int properties() = 0;
    virtual QStringList compatible() = 0;

    // return 0 if you don't have one.
    virtual QWidget *inputWidget( QWidget *parent, Qt::WFlags f ) = 0;
    virtual QWSInputMethod *inputModifier() = 0;
    virtual QWSGestureMethod *gestureModifier() = 0;

    virtual void qcopReceive( const QCString &msg, const QByteArray &data ) = 0;

    // Guess functions for cooperating input methods.
    // Only used if one provides guesses, and the other modifies them.
    // used to communicate between plugins.
    //virtual void appendGuess(const QChar &, const QString &) = 0;
    virtual void appendGuess(const IMIGuessList &) = 0;
    virtual void revertGuess() = 0;
    virtual void function(const QString &) = 0;

    // signiture is "(const QChar&, const QString&)"
    // best guess, other guesses
    virtual void connectAppendGuess(QObject *receiver, const char *slot) = 0;
    virtual void disconnectAppendGuess(QObject *receiver, const char *slot) = 0;

    // signiture is "()"
    virtual void connectRevertGuess(QObject *receiver, const char *slot) = 0;
    virtual void disconnectRevertGuess(QObject *receiver, const char *slot) = 0;

    // signiture is "(const QString &)"
    virtual void connectFunction(QObject *receiver, const char *slot) = 0;
    virtual void disconnectFunction(QObject *receiver, const char *slot) = 0;

    // probably should be dropped as a function.
    virtual void setHint(const QString &) = 0;

    // This should be something like 'statusWidgetRequired'.  which gets merged into the
    // state 'Sleeping'.
    virtual bool restrictToHint() const;
    virtual void setRestrictToHint(bool);
};

#endif
