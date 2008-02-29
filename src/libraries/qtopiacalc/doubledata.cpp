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
#include "doubledata.h"
#include "engine.h"
#include <qstring.h>

// Data type
DoubleData::DoubleData(): Data() {};

void DoubleData::set(double d) {
    dbl = d;
    edited = FALSE;
    formattedOutput.setNum(dbl,'g',16);
    if (!strcmp(formattedOutput.latin1(),"nan")) { // No tr
	systemEngine->setError(eNotANumber);
	return;
    } else if (!strcmp(formattedOutput.latin1(),"inf")) { // No tr
	systemEngine->setError(eInf);
	return;
    }
}
bool DoubleData::push(char c, bool commit) {
    if (formattedOutput.length() > 15 || 
	    (formattedOutput == "0" && c == '0'))
	return FALSE;
    QString tmpString = formattedOutput;
    if (!edited) {
	if (c == '.') 
	    tmpString = QString("0");
	else
	    tmpString.truncate(0);
	edited = TRUE;
    }
    tmpString.append(c);
    bool ok;
    double tmp = tmpString.toDouble(&ok);
    if (ok) {
	if (commit) {
	    formattedOutput = tmpString;
	    dbl = tmp;
	}
    } else
	qDebug("Wrong character pushed");
    return ok;
}
void DoubleData::del() {
    if (!edited)
	return;
    if (formattedOutput.length() == 1) {
	formattedOutput.truncate(0);
	formattedOutput.append("0");
	edited = FALSE;
	dbl = 0;
    } else {
	QString tmpString = formattedOutput;
	tmpString.truncate(formattedOutput.length()-1);
	bool ok;
	double tmp = tmpString.toDouble(&ok);
	if (ok) {
	    formattedOutput = tmpString;
	    dbl = tmp;
	}
    }
}
void DoubleData::clear() {
    dbl = 0;
    formattedOutput.truncate(0);
    formattedOutput.append("0");
    edited = FALSE;
}
