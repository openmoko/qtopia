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

#include "qabstractmessagebox.h"
#include "qtopiaserverapplication.h"

#include <QBasicTimer>

#ifndef Q_WS_X11
#  include <qtopia/private/testslaveinterface_p.h>
#  define QTOPIA_USE_TEST_SLAVE 1
#endif

// declare QAbstractMessageBoxPrivate
class QAbstractMessageBoxPrivate : public QObject
{
    Q_OBJECT
public:
    QAbstractMessageBoxPrivate(int time, QAbstractMessageBox::Button but,
                               QObject *parent)
    : QObject(parent), button(but), timeout(time)
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
        if (timeout > 0)
            timer.start(timeout, this);
    }

    void endTimeout()
    {
        timer.stop();
    }

signals:
    void done(int);

protected:
    virtual void timerEvent(QTimerEvent *e)
    {
        if (timer.timerId() == e->timerId()) {
            endTimeout();
            emit done(button);
        }
    }

private:
    QAbstractMessageBox::Button button;
    int timeout;
    QBasicTimer timer;
};

/*!
  \class QAbstractMessageBox
  \brief The QAbstractMessageBox class allows developers to replace the message box portion of the Qtopia server UI.
  \ingroup QtopiaServer
  \ingroup QtopiaServer::PhoneUI::TTSmartPhone

  The abstract message box is part of the 
  \l {QtopiaServerApplication#qtopia-server-widgets}{server widgets framework}
  and represents the portion of the Qtopia server UI that is shown to users 
  when a message box is needed.
  
  A small tutorial on how to develop new server widgets using one of the 
  abstract widgets as base can be found in QAbstractServerInterface class 
  documentation.

  The QAbstractMessageBox API is intentionally designed to be similar to the 
  QMessageBox API to facilitate easily replacing one with the other.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
  */

/*!
  \enum QAbstractMessageBox::Icon

  Represents the icon to show in the message box.

  \value NoIcon No icon will be shown.
  \value Question A question icon will be shown.
  \value Information An information icon will be shown.
  \value Warning A warning icon will be shown.
  \value Critical A critical condition icon will be shown.
 */

/*!
  \enum QAbstractMessageBox::Button

  Represents a standard button that may be displayed on the message box.

  \value NoButton An empty button.  This indicates that no button is required.
  \value Ok An Ok button.
  \value Cancel A cancel button.
  \value Yes A yes button.
  \value No A no button.
 */

/*!
  \fn void QAbstractMessageBox::setButtons(Button button1, Button button2)

  Sets the buttons on the message box to the standard buttons \a button1 and
  \a button2.
  */

/*!
  \fn void QAbstractMessageBox::setButtons(const QString &button0Text, const QString &button1Text, const QString &button2Text, int defaultButtonNumber, int escapeButtonNumber)

  Sets the buttons on the message box to custom buttons.

  Up to three custom buttons may be specified with \a button0Text, 
  \a button1Text and \a button2Text which are mapped to keys in a system 
  specific way.  The exec() return value for each of these buttons will be
  0, 1 or 2 repspectively.

  The \a defaultButtonNumber id is returned from exec if the dialog is simply
  "accepted" - usually by pressing the select key.  Likewise the 
  \a escapeButtonNumber is returned if the dialog is dismissed.
  */

/*!
  \fn QString QAbstractMessageBox::title() const

  Returns the title text of the message box.
  */

/*!
  \fn void QAbstractMessageBox::setTitle(const QString &title)

  Sets the \a title text of the message box.
  */

/*!
  \fn QAbstractMessageBox::Icon QAbstractMessageBox::icon() const

  Returns the message box icon.
  */

/*!
  \fn void QAbstractMessageBox::setIcon(Icon icon)

  Sets the message box \a icon.
  */

/*!
  \fn QString QAbstractMessageBox::text() const

  Returns the message box text.
  */

/*!
  \fn void QAbstractMessageBox::setText(const QString &text)

  Sets the message box \a text.
  */

/*!
  Constructs a new QAbstractMessageBox instance, with the specified
  \a parent and widget \a flags.
  */
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

/*!
  Opens a critical message box with the \a title and \a text.  The standard
  buttons \a button1 and \a button2 are added to the message box.

  Returns the identity of the standard button that was activated.

  If \a parent is 0, the message box becomes an application-global message box.
  If \a parent is a widget, the message box becomes modal relative to
  \a parent.
 */
int QAbstractMessageBox::critical(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Critical, title, text, button1, button2);
}

/*!
  Opens a warning message box with the \a title and \a text.  The standard
  buttons \a button1 and \a button2 are added to the message box.

  Returns the identity of the standard button that was activated.

  If \a parent is 0, the message box becomes an application-global message box.
  If \a parent is a widget, the message box becomes modal relative to
  \a parent.
 */
int QAbstractMessageBox::warning(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Warning, title, text, button1, button2);
}

/*!
  Opens an informational message box with the \a title and \a text.  The 
  standard buttons \a button1 and \a button2 are added to the message box.

  Returns the identity of the standard button that was activated.

  If \a parent is 0, the message box becomes an application-global message box.
  If \a parent is a widget, the message box becomes modal relative to
  \a parent.
 */
int QAbstractMessageBox::information(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Information, title, text, button1, button2);
}

/*!
  Opens a question message box with the \a title and \a text.  The standard
  buttons \a button1 and \a button2 are added to the message box.

  Returns the identity of the standard button that was activated.

  If \a parent is 0, the message box becomes an application-global message box.
  If \a parent is a widget, the message box becomes modal relative to
  \a parent.
 */
int QAbstractMessageBox::question(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Question, title, text, button1, button2);
}

/*!
  Returns a new message box instance with the specified \a parent, \a title,
  \a text, \a icon and standard buttons \a button0 and \a button1.
 */
QAbstractMessageBox *QAbstractMessageBox::messageBox(QWidget *parent, const QString &title, const QString &text, Icon icon, Button button0, Button button1)
{
    QAbstractMessageBox *box = qtopiaWidget<QAbstractMessageBox>(parent);
    box->setIcon(icon);
    box->setTitle(title);
    box->setText(text);
    box->setButtons(button0, button1);
    return box;
}

/*!
  Returns a new custom message box instance with the specified \a parent, 
  \a title, \a text and \a icon. 
  
  Up to three custom buttons may be specified with \a button0Text, 
  \a button1Text and \a button2Text which are mapped to keys in a system 
  specific way.  The exec() return value for each of these buttons will be
  0, 1 or 2 repspectively.

  The \a defaultButtonNumber id is returned from exec if the dialog is simply
  "accepted" - usually by pressing the select key.  Likewise the 
  \a escapeButtonNumber is returned if the dialog is dismissed.
 */
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

/*!
  Set an auto timeout value of \a timeoutMs milliseconds.  The dialog will be
  automatically accepted as though the user pressed the \a button key after
  this time.
 */
void QAbstractMessageBox::setTimeout(int timeoutMs, Button button)
{
    if(timeoutMs) {
        if(d) {
            d->changeTimeout(timeoutMs, button);
        } else {
            d = new QAbstractMessageBoxPrivate(timeoutMs, button, this);
            connect( d, SIGNAL(done(int)), this, SLOT(done(int)) );
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


/*! \internal */
void QAbstractMessageBox::hideEvent(QHideEvent *e)
{
    if(d)
        d->endTimeout();

    QDialog::hideEvent(e);
}

/*! \internal */
void QAbstractMessageBox::showEvent(QShowEvent *e)
{
    if(d)
        d->startTimeout();

#ifdef QTOPIA_USE_TEST_SLAVE
    if (QtopiaApplication::instance()->testSlave()) {
        QVariantMap map;
        map["title"] = windowTitle();
        map["text"] = text();
        QtopiaApplication::instance()->testSlave()->postMessage("show_messagebox", map);
    }
#endif

    QDialog::showEvent(e);
}

#include "qabstractmessagebox.moc"
