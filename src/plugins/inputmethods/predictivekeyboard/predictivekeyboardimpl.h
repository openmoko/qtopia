/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef PREDKEYBOARDIMPL_H
#define PREDKEYBOARDIMPL_H

#include <QtopiaInputMethod>
#include <QWidget>
//#include <inputmethod.h>
#include <QTimeLine>
#include <QTextEdit>

#include <QDebug>

#include "pred.h"
#include "predictivekeyboard.h"

class QPaintEvent;
class QMouseEvent;

class PredictiveKeyboard;
class PredictiveKeyboardWidget;
class WordPredict;
class QPaintEvent;


class PredictiveKeyboardInputMethod : public QtopiaInputMethod
{
    Q_OBJECT
public:
    PredictiveKeyboardInputMethod(QObject * =0);
    ~PredictiveKeyboardInputMethod();

    QString name() const;
    QString identifier() const;
    QString version() const;

    State state() const;
    int properties() const { return RequireMouse | InputModifier | DockedInputWidget; } 
    QIcon icon() const;

    void setHint(const QString &, bool );

    // state managment.
    void reset();

    QWidget *inputWidget( QWidget* parent = 0);
    QWSInputMethod *inputModifier();

    void menuActionActivated(int v);
    QList<QIMActionDescription*> menuDescription();

private slots:
    //void sendKeyPress( ushort unicode, ushort keycode, ushort modifiers, bool press, bool repeat );
private:
    friend class PredictiveKeyboard;
    friend class PredictiveKeyboardWidget;

    PredictiveKeyboard *input;

    void queryResponse ( int property, const QVariant & result );
};


#endif
