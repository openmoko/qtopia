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

#include "qabstractmessagebox.h"
#include "qtopiaserverapplication.h"

// declare QAbstractMessageBoxPrivate
class QAbstractMessageBoxPrivate : public QObject
{
    Q_OBJECT
public:
    QAbstractMessageBoxPrivate(int time, QAbstractMessageBox::Button but,
                               QObject *parent)
    : QObject(parent), button(but), timeout(time), timerId(0)
    {
    }

    void changeTimeout(int time, QAbstractMessageBox::Button but)
    {
        endTimeout();
        timeout = time;
        button = but;
    }

    void startTimeout()
    {
        if(!timerId) {
            timerId = startTimer(timeout);
        }
    }

    void endTimeout()
    {
        if(timerId) {
            killTimer(timerId);
            timerId = 0;
        }
    }

signals:
    void done(int);

protected:
    virtual void timerEvent(QTimerEvent *)
    {
        endTimeout();
        emit done(button);
    }

private:
    QAbstractMessageBox::Button button;
    int timeout;
    int timerId;
};

QAbstractMessageBox::QAbstractMessageBox(QWidget *parent,
                                         Qt::WFlags flags)
: QDialog(parent, flags), d(0)
{
}

static int QAbstractMessageBox_exec(QWidget *parent, QAbstractMessageBox::Icon icon, const QString &title, const QString &text, QAbstractMessageBox::Button button1, QAbstractMessageBox::Button button2)
{
    QAbstractMessageBox *box = qtopiaWidget<QAbstractMessageBox>(parent);
    box->setIcon(icon);
    box->setTitle(title);
    box->setText(text);
    box->setButtons(button1, button2);
    int rv = QtopiaApplication::execDialog(box);
    delete box;
    return rv;
}

int QAbstractMessageBox::critical(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Critical, title, text, button1, button2);
}

int QAbstractMessageBox::warning(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Warning, title, text, button1, button2);
}

int QAbstractMessageBox::information(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Information, title, text, button1, button2);
}

int QAbstractMessageBox::question(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Question, title, text, button1, button2);
}

QAbstractMessageBox *QAbstractMessageBox::messageBox(QWidget *parent, const QString &title, const QString &text, Icon icon, Button button0, Button button1)
{
    QAbstractMessageBox *box = qtopiaWidget<QAbstractMessageBox>(parent);
    box->setIcon(icon);
    box->setTitle(title);
    box->setText(text);
    box->setButtons(button0, button1);
    return box;
}

QAbstractMessageBox *QAbstractMessageBox::messageBoxCustomButton(QWidget *parent, const QString &title, const QString &text, Icon icon,
            const QString & button0Text, const QString &button1Text,
            const QString &button2Text, int defaultButtonNumber, int escapeButtonNumber)
{
    QAbstractMessageBox *box = qtopiaWidget<QAbstractMessageBox>(parent);
    box->setIcon(icon);
    box->setTitle(title);
    box->setText(text);
    box->setButtons(button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
    return box;
}

void QAbstractMessageBox::setTimeout(int timeoutMs, Button button)
{
    if(timeoutMs) {
        if(d) {
            d->changeTimeout(timeoutMs, button);
        } else {
            d = new QAbstractMessageBoxPrivate(timeoutMs, button, this);
        }

        if(!isHidden())
            d->startTimeout();
    } else {
        if(d) {
            delete d;
            d = 0;
        }
    }
}

void QAbstractMessageBox::hideEvent(QHideEvent *e)
{
    if(d)
        d->endTimeout();

    QDialog::hideEvent(e);
}

void QAbstractMessageBox::showEvent(QShowEvent *e)
{
    if(d)
        d->startTimeout();

    QDialog::showEvent(e);
}

#include "qabstractmessagebox.moc"

