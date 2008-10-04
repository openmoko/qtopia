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

#include "minsecspinbox.h"

#include <QDebug>

/*
    A spin box displaying length of time in minutes and seconds
*/

MinSecSpinBox::MinSecSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    lineEdit()->setReadOnly(true);
    //TODO: ideally, disable or hide selection as well
}

MinSecSpinBox::~MinSecSpinBox()
{}

QString MinSecSpinBox::textFromValue(int value) const
{
    if (value == minimum())
        return tr("Off");
    
    if (value > 59) {
        int m = value/60;
        int s = value%60;
        QString str = (s == 0 ? QString("%1m").arg(m) : QString("%1m%2s").arg(m).arg(s));
        return str;
    } else {
        return QString("%1s").arg(value);
    }
}

int MinSecSpinBox::valueFromText(const QString& text) const
{
    if (text == tr("Off"))
        return minimum();
    QRegExp regExp("((\\d+)m)?((\\d+)s?)?");
    if (regExp.exactMatch(text)) {
        int s = regExp.cap(2).toInt()*60 + regExp.cap(4).toInt();
        return s;
    } else {
        return 0;
    }
}

// normal sizeHint doesn't always work right for our case, 
// because the min/max/special text aren't necessarily the longest text
QSize MinSecSpinBox::sizeHint() const
{
    QSize normal = QSpinBox::sizeHint();
    
    //this first portion copied from QAbstractSpinBox
    const QFontMetrics fm(fontMetrics());
    int w = 0;
    QString s;
    s = prefix() + textFromValue(minimum()) + suffix() + QLatin1Char(' ');
    s.truncate(18);
    w = qMax(w, fm.width(s));
    s = prefix() + textFromValue(maximum()) + suffix() + QLatin1Char(' ');
    s.truncate(18);
    w = qMax(w, fm.width(s));
    if (specialValueText().size()) {
        s = specialValueText();
        w = qMax(w, fm.width(s));
    }
    w += 2; // cursor blinking space
    
    //new part (see if max - singlestep is longer)
    s = textFromValue(maximum()-singleStep()) + QLatin1Char(' ');
    int longwidth = fm.width(s) + 2;
    int dif = longwidth - w;
    if (dif > 0)
        normal += QSize(dif, 0);
    
    return normal;
}
