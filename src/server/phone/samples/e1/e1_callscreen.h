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

#ifndef _e1_CALLSCREEN_H_
#define _e1_CALLSCREEN_H_

#include <QWidget>
#include <QDateTime>
#include <QPixmap>
#include <QString>
class QLabel;
class QPhoneCall;
class E1Button;

class E1Callscreen : public QWidget
{
Q_OBJECT
public:
    E1Callscreen(E1Button *b, QWidget *parent);

    void setActive();

signals:
    void showMe();
    void closeMe();
    void toDialer();

public slots:
    void sendNumber(const QString &);

private slots:
    void callConnected( const QPhoneCall &call );
    void callIncoming( const QPhoneCall &call );
    void callDialing( const QPhoneCall &call );
    void callEnded(const QPhoneCall &call);

    void mute();
    void spkr();
    void hold();
    void end();

protected:
    virtual void timerEvent(QTimerEvent *);
    virtual void paintEvent(QPaintEvent *);

private:
    void updateCallTime();
    void updateInfo(const QPhoneCall &);
    int m_timer;
    QString m_hasCall;

    QString m_name;
    QPixmap m_image;

    QString m_state;
    QString m_time;

    QDateTime m_callTime;
    QLabel *nameLabel;
    QLabel *imageLabel;
    QLabel *timeLabel;
    QLabel *stateLabel;
    E1Button *m_button;
};

#endif // _e1_CALLSCREEN_H_
