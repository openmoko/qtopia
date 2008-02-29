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

#ifndef DIALER_H
#define DIALER_H

#include <themedview.h>
#include <quniqueid.h>

#include <QDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include "phonethemeview.h"
#include "qabstractdialerscreen.h"

class Dialer;
class PhoneTouchDialerScreen : public QAbstractDialerScreen
{
Q_OBJECT
public:
    PhoneTouchDialerScreen(QWidget *parent = 0, Qt::WFlags f = 0);

    virtual QString digits() const;
    virtual void reset();
    virtual void appendDigits(const QString &digits);
    virtual void setDigits(const QString &digits);

protected slots:
    void keyEntered(const QString &key);

private:
    Dialer *m_dialer;
};

#endif
