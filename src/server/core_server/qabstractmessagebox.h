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

#ifndef _QABSTRACTMESSAGEBOX_H_
#define _QABSTRACTMESSAGEBOX_H_

#include <QDialog>
#include "qtopiaserverapplication.h"

class QAbstractMessageBoxPrivate;
class QAbstractMessageBox : public QDialog
{
Q_OBJECT
public:
    enum Icon { NoIcon=0, Question=1, Information=2, Warning=3, Critical=4 };
    enum Button { NoButton=0, Ok=11, Cancel=12, Yes=13, No=14 };

    QAbstractMessageBox(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual void setButtons(Button button1, Button button2) = 0;
    virtual void setButtons(const QString &button0Text, const QString &button1Text, const QString &button2Text,
            int defaultButtonNumber, int escapeButtonNumber) = 0;

    virtual QString title() const = 0;
    virtual void setTitle(const QString &) = 0;

    virtual Icon icon() const = 0;
    virtual void setIcon(Icon) = 0;

    virtual QString text() const = 0;
    virtual void setText(const QString &) = 0;

    virtual void setTimeout(int timeoutMs, Button);

    static int critical(QWidget *parent, const QString &title, const QString &text, Button button1 = NoButton, Button button2 = NoButton);
    static int warning(QWidget *parent, const QString &title, const QString &text, Button button1 = NoButton, Button button2 = NoButton);
    static int information(QWidget *parent, const QString &title, const QString &text, Button button1 = NoButton, Button button2 = NoButton);
    static int question(QWidget *parent, const QString &title, const QString &text, Button button1 = NoButton, Button button2 = NoButton);

    static QAbstractMessageBox * messageBox(QWidget *parent, const QString &title, const QString &text, Icon icon, Button button0=QAbstractMessageBox::Ok, Button button1=QAbstractMessageBox::NoButton);
    static QAbstractMessageBox * messageBoxCustomButton(QWidget *parent, const QString &title, const QString &text, Icon icon,
            const QString & button0Text = QString(), const QString &button1Text = QString(),
            const QString &button2Text = QString(), int defaultButtonNumber = 0, int escapeButtonNumber = -1 );

protected:
    virtual void hideEvent(QHideEvent *);
    virtual void showEvent(QShowEvent *);

private:
    QAbstractMessageBoxPrivate *d;
};

#endif // _QABSTRACTMESSAGEBOX_H_
