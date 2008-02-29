/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
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
    // setNum uses sprintf. %g option is not appropriate to use -> change to f option
    // any precision >13 will produce slightly erroneous result => bug in sprintf
    formattedOutput.setNum(dbl,'f',13);
    
    int point = formattedOutput.find('.');
    if (point > 11)
    {   
        QRegExp reg = QRegExp("[1..9]+");
        if (point > 12)
            systemEngine->setError(eSurpassLimits);
        else if ( point == 12 && formattedOutput.mid(13).contains(reg) ) {
            systemEngine->setError(eSurpassLimits);
        }
    }
    
    formattedOutput.truncate(12);
    //remove trailing zeros if decimal point is present
    if (formattedOutput.find('.') > -1){
    int i = formattedOutput.length()-1;
    int max = i + 1;
    while (formattedOutput.at(i) == '0')
        i--;
    formattedOutput.remove(++i, max-i);
    
    if (formattedOutput.at( i-1 ) == '.')
	formattedOutput.remove( i-1 , 1 );
    }
    
    if (!strcmp(formattedOutput.latin1(),"nan")) { // No tr
	systemEngine->setError(eNotANumber);
    } else if (!strcmp(formattedOutput.latin1(),"inf")) { // No tr
	systemEngine->setError(eInf);
    } else if (!strcmp(formattedOutput.latin1(),"-inf")) { // No tr
	systemEngine->setError(eNegInf);
    }
}

double DoubleData::get() { return dbl; }

bool DoubleData::push(char c, bool commit) {
    if (edited && formattedOutput.length() >= 12)
	return FALSE;
    // Allow zero to be input as a value, but only once
    if (formattedOutput == "0" && c == '0')
	return !edited;

    //when +/- is pressed while no number has been entered
    //return to !edited mode
    if (formattedOutput == "0" && edited) 
        edited = !edited;

    QString tmpString = formattedOutput;
    if (!edited) {
	if (c == '.') 
	    tmpString = QString("0");
	else
	    tmpString.truncate(0);
	// Dont change the value of edited on the test run
	if (commit)
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
bool DoubleData::del() {
    if (!edited)
	return TRUE;
    if (formattedOutput.length() == 1) {
	formattedOutput.truncate(0);
	formattedOutput.append("0");
	edited = FALSE;
	dbl = 0;
        return TRUE;
    } else {
	QString tmpString = formattedOutput;
	tmpString.truncate(formattedOutput.length()-1);
	bool ok;
	double tmp = tmpString.toDouble(&ok);
	if (ok) {
	    formattedOutput = tmpString;
	    dbl = tmp;
	} else
            return TRUE;
    }
    return FALSE;
}
void DoubleData::clear() {
    dbl = 0;
    formattedOutput.truncate(0);
    formattedOutput.append("0");
    edited = FALSE;
}
