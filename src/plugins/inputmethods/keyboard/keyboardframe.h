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
#ifndef KEYBOARDFRAME_H
#define KEYBOARDFRAME_H
#include "pickboardcfg.h"
#include "pickboardpicks.h"
#include <QDebug>

class QTimer;

enum currentPosition
{
    Top,
    Bottom
};

class KeyboardConfig : public DictFilterConfig
{
public:
    KeyboardConfig(PickboardPicks* p) : DictFilterConfig(p), backspaces(0) { nrows = 1; }
    virtual ~KeyboardConfig();
    virtual void generateText(const QString &s);
    void decBackspaces() { if (backspaces) backspaces--; }
    void incBackspaces() { backspaces++; }
    void resetBackspaces() { backspaces = 0; }
private:
    int backspaces;
};


class KeyboardPicks : public PickboardPicks
{
    Q_OBJECT
public:
    KeyboardPicks(QWidget* parent=0, Qt::WFlags f=0)
        : PickboardPicks(parent, f), dc(0) { }
    virtual ~KeyboardPicks();
    void initialise();
    virtual QSize sizeHint() const;
    KeyboardConfig *dc;
};

/*
    KeyboardFrame is the primary widget for the Keyboard inputmethod.
    It is responsible for marshalling pickboards for displaying the pickboard,
    and for handling mouseevents.

    It currently also creates and dispatches keyevents, although this is
    expected to be re-routed through Keyboard in the future.
*/

class KeyboardFrame : public QFrame // was QFrame
{
    Q_OBJECT
public:
    KeyboardFrame( QWidget* parent=0, Qt::WFlags f=0 );
    virtual ~KeyboardFrame();

    void resetState();

    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void resizeEvent(QResizeEvent*);
    void showEvent(QShowEvent*);
    void paintEvent(QPaintEvent* e);
    void timerEvent(QTimerEvent* e);
    void drawKeyboard(QPainter &p, int key = -1 );

    void hideEvent ( QHideEvent * );

    void setMode(int mode) { useOptiKeys = mode; }

    QSize sizeHint() const;

    bool obscures( const QPoint &point );

    void focusInEvent ( QFocusEvent *e){
    Q_UNUSED(e);
    qWarning() << "Warning: keyboard got focus";
    };

    bool filter(int /*unicode*/, int /*keycode*/, int /*modifiers*/,
                        bool /*isPress*/, bool /*autoRepeat*/){return false;};

    bool filter(const QPoint &, int /*state*/, int /*wheel*/){return false;};

signals:
    void needsPositionConfirmation();
    void hiding();
    void showing();

public slots:
    void swapPosition();

protected:
//    void reset(){QWSInputMethod::reset();};
//    void updateHandler(int type){};
//    void mouseHandler(int pos, int state){};
//    void queryResponse(int property, const QVariant&){};


private slots:
    void repeat();

private:

    int getKey( int &w, int j = -1 );
    void clearHighlight();

    uint shift:1;
    uint lock:1;
    uint ctrl:1;
    uint alt:1;
    uint useLargeKeys:1;
    uint useOptiKeys:1;

    int pressedKey;

    KeyboardPicks *picks;

    int keyHeight;
    int defaultKeyWidth;
    int xoffs;

    int unicode;
    int qkeycode;
    Qt::KeyboardModifiers modifiers;

    int pressTid;
    bool pressed;

    bool microFocusPending;
    bool showPending;

    bool positionTop;

    QTimer *repeatTimer;
};

#endif //KEYBOARDFRAME_H
