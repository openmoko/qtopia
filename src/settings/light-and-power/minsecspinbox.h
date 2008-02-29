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

#ifndef MINSECSPINBOX_H
#define MINSECSPINBOX_H

#include <QtGui>

class MinSecSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit MinSecSpinBox(QWidget *parent = 0);
    ~MinSecSpinBox();
    
    QSize sizeHint() const;

protected:
    QString textFromValue(int value) const;
    int valueFromText(const QString& text) const;
};

#endif
