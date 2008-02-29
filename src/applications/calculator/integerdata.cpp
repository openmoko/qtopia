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
#include "integerdata.h"
#include <QDebug>

// Data type functions
void IntegerData::set(int j,int b) {
    i = j;
    if (base != 2 || base != 8 ||
            base != 10 || base != 16) {
        qWarning("Base %d is not supported",b);
        base = 10;
    } else
        base = b;
}
bool IntegerData::appendChar(char c) {
    QString tmp = formattedOutput;
    tmp.append(c);
    bool ok;
    i = tmp.toInt(&ok,base);
    if (!ok)
        return false;
    formattedOutput = tmp;
    return true;
}
void IntegerData::delChar() {
    formattedOutput.truncate(formattedOutput.length()-1);
    if (!formattedOutput.length())
        formattedOutput.setNum(0);
    bool ok;
    i = formattedOutput.toInt(&ok,base);
}

