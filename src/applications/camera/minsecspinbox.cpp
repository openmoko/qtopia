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

#include "minsecspinbox.h"

#include <QDebug>

/*
    A spin box displaying length of time in minutes and seconds
*/

CameraMinSecSpinBox::CameraMinSecSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    lineEdit()->setReadOnly(true);
    //TODO: ideally, disable or hide selection as well
}

CameraMinSecSpinBox::~CameraMinSecSpinBox()
{}

QString CameraMinSecSpinBox::textFromValue(int value) const
{
    if (value == minimum())
        return tr("Off");
    
    if (value > 59) {
        int m = value/60;
        int s = value%60;
        QString str = (s == 0 ? tr("%1m", "5 minutes").arg(m) : tr("%1m%2s","5minutes 4 seconds").arg(m).arg(s));
        return str;
    } else {
        return tr("%1s","5 seconds").arg(value);
    }
}

int CameraMinSecSpinBox::valueFromText(const QString& text) const
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
QSize CameraMinSecSpinBox::sizeHint() const
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
