/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
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
