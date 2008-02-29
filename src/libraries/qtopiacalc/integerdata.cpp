/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "integerdata.h"

// Data type functions
void IntegerData::set(int j,int b) {
    i = j;
    if (base != 2 || base != 8 ||
	    base != 10 || base != 16) {
	qDebug("Base %d is not supported",b);
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
	return FALSE;
    formattedOutput = tmp;
    return TRUE;
}
void IntegerData::delChar() {
    formattedOutput.truncate(formattedOutput.length()-1);
    if (!formattedOutput.length())
	formattedOutput.setNum(0);
    bool ok;
    i = formattedOutput.toInt(&ok,base);
}

