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

#ifndef _e2_DIALER_H_
#define _e2_DIALER_H_

#include <QWidget>
#include <QPixmap>
#include <QLineEdit>
#include <QList>
class E2Button;
class QGridLayout;
class E2DialerButton;
class E2CallHistory;
class E2Dialer : public QWidget
{
Q_OBJECT
public:
    E2Dialer(E2Button *b, QWidget *parent = 0, Qt::WFlags flags = 0);

    void setActive();
    void setNumber(const QString &);

signals:
    void sendNumber(const QString &);
    void toCallscreen();

private slots:
    void showHistory();
    void backspace();
    void number();
    void hash();
    void star();
    void textChanged();
    void callNow();
    void activeCallCount(int c);

private:
    void addDtmf(const QString &);

    int m_activeCallCount;
    QLineEdit *m_lineEdit;
    QList<QObject *> m_numbers;
    E2Button *m_button;
    E2DialerButton *m_callscreen;
    QGridLayout *m_grid;
    E2CallHistory *m_history;
};

class E2DialerButton : public QWidget
{
Q_OBJECT
public:
    E2DialerButton(const QPixmap &, bool highlight = false, QWidget *parent = 0);

signals:
    void clicked();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

private:
    bool m_highlight;
    bool m_pressed;
    QPixmap m_button;
};

#endif // _e2_DIALER_H_

