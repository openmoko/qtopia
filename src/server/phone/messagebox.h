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

#ifndef MESSAGE_BOX_H
#define MESSAGE_BOX_H

#include <qdialog.h>
#include "qabstractmessagebox.h"

class PhoneMessageBoxPrivate;
class PhoneMessageBox : public QAbstractMessageBox
{
Q_OBJECT
public:
    PhoneMessageBox(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual void setButtons(Button button1, Button button2);
    virtual void setButtons(const QString &button0Text, const QString &button1Text, const QString &button2Text,
            int defaultButtoNumber, int escapeButtonNumber);
    virtual QString title() const;
    virtual void setTitle(const QString &);
    virtual Icon icon() const;
    virtual void setIcon(Icon);
    virtual QString text() const;
    virtual void setText(const QString &);

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    void addContents(QWidget *);

private:
    PhoneMessageBoxPrivate *d;
};

#endif

