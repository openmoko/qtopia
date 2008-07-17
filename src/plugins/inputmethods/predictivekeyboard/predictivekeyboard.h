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
#ifndef predictivekeyboard_H
#define predictivekeyboard_H

#include <qtopiaipcenvelope.h>
#include <QDebug> //tmp

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#elif defined(Q_WS_X11)
#include "qwsinputmethod_x11.h"
#endif

/*
    PredictiveKeyboard is an input method for Qtopia.  PredictiveKeyboard displays a popup widget depiciting keys onscreen (PredictiveKeyboardWidget), and converts them into key events.

*/

class QAction;
class KeyboardWidget;
class PredictiveKeyboard : public QWSInputMethod
{
    Q_OBJECT
public:
    PredictiveKeyboard(QWidget* parent = 0);
    virtual ~PredictiveKeyboard();

    void queryResponse ( int property, const QVariant & result );

    QWidget* widget(QWidget *parent = 0);
    void resetState();
    QAction* menuActionToDuplicate(){ return mAction;};
    virtual bool filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);
    // Suppress hidden function warnings
    virtual inline bool filter( const QPoint&, int, int ) {return false;}

signals:
    void stateChanged();

public slots:
    void erase();
    void submitWord(QString word);
    void preedit(QString text);
#ifdef Q_WS_QWS
    void checkMicroFocus();
    void windowEvent(QWSWindow *w, QWSServer::WindowEvent e);
#endif

protected:
#ifdef Q_WS_QWS
    virtual void updateHandler(int type);
#endif

private:
    friend class PredictiveKeyboardInputMethod;
    QAction* mAction;
    KeyboardWidget *mKeyboard;
#ifdef Q_WS_QWS
    QWSWindow *mActive;
#endif
};

#endif //predictivekeyboard_H
