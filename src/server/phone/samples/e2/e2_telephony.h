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

#ifndef _E2_TELEPHONY_H_
#define _E2_TELEPHONY_H_

#include <QWidget>
class QString;
class QByteArray;
class E2CallScreen;
class E2Dialer;
class E2Bar;
class E2TelephonyBar;

class E2Telephony : public QWidget
{
Q_OBJECT
public:
    E2Telephony(QWidget *parent = 0);


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

    E2TelephonyBar *m_tbar;
    E2CallScreen *m_callscreen;
    E2Dialer *m_dialer;
    E2Bar *m_bar;
};

#endif // _E2_TELEPHONY_H_


