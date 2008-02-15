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

#ifndef _e2_HEADER_H_
#define _e2_HEADER_H_

#include <QWidget>
#include <QPixmap>
#include <QStringList>
#include <QList>
#include <QValueSpaceItem>

class E2HeaderButton;
class E2AlertScreen;
class E2Header : public QWidget
{
Q_OBJECT
public:
    E2Header(QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *);

private slots:
    void alertClicked();
    void clicked(const QString &name);
    void activeCallCount(int);
    void setAlertEnabled(bool e);

private:
    QPixmap m_fillBrush;
    QStringList m_alertStack;
    QList<E2HeaderButton *> m_buttons;
    E2HeaderButton *m_alert;
    E2AlertScreen *m_alertScreen;
    E2HeaderButton *m_phone;
    E2HeaderButton *m_phoneActive;
};


#endif // _e2_HEADER_H_
