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

#ifndef _e1_TELEPHONY_H_
#define _e1_TELEPHONY_H_

#include <QWidget>
class QString;
class QByteArray;
class E1Callscreen;
class E1Dialer;
class E1Bar;
class E1PhoneTelephonyBar;

class E1Telephony : public QWidget
{
Q_OBJECT
public:
    E1Telephony(QWidget *parent = 0);


public slots:
    void display();
    void popupDialer();
    void popupCallscreen();
    void slideToDialer();
    void slideToCallscreen();

protected:
    virtual void showEvent(QShowEvent *);
    virtual void timerEvent(QTimerEvent *);

private slots:
    void message(const QString &message, const QByteArray &);
    void doShowDialer(const QString &);

private:
    enum { CallScreen, Dialer } m_state;

    void doLayout();

    unsigned int m_scrollStep;
    static const unsigned int m_scrollSteps = 10;
    static const unsigned int m_scrollStepTime = 20;

    E1PhoneTelephonyBar *m_tbar;
    E1Callscreen *m_callscreen;
    E1Dialer *m_dialer;
    E1Bar *m_bar;
};

#endif // _e1_TELEPHONY_H_


