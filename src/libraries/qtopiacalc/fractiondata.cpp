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
#include "fractiondata.h"
#include "engine.h"
#include <qstring.h>

// Data type functions
bool FractionData::push(char c, bool commit) {
    if (formattedOutput.length() > 15)
	return FALSE;
    if ( numerator == 0 && c == '0' )
        return !edited;
    if (edited && numerator == 0)
        edited = FALSE;
    
    if (!edited) {
        numerator = 0;
        nString.truncate(0);
        dString.truncate(0);
        if (commit)
            edited = TRUE;
            dEdited = FALSE;
    }
        
    if (c == '/') {
	if (dEdited) 
	    return FALSE;
	if (commit) {
            if (numerator == 0){
                numerator = 1;
                nString.setNum(1);
            }
	    denominator = 0;
	    dEdited = TRUE;
            edited = TRUE;
            dString.truncate(0);
	    buildFormattedString();
	}
	return TRUE;
    }

    // append char and test result
    bool ok = FALSE;
    QString tmpString;
    int num;
    if (dEdited) {
	tmpString = dString;
	tmpString.append(c);
	num = tmpString.toInt(&ok);
	if (ok && commit) {
	    dString = tmpString;
	    denominator = num;
	}
    } else {
	tmpString = nString;
	tmpString.append(c);
	num = tmpString.toInt(&ok);
	if (ok && commit) {
	    nString = tmpString;
	    numerator = num;
            edited = TRUE;
	}
    }
    if (ok && commit)
	buildFormattedString();
    return ok;
}
// puts dString and nString together
void FractionData::buildFormattedString() {
    formattedOutput.truncate(0);
    if (denominator < 0) {
	numerator = 0 - numerator;
	nString.truncate(0);
	nString.setNum(numerator);
	denominator = 0 - denominator;
	dString.truncate(0);
	dString.setNum(denominator);
	buildFormattedString();
	return;
    }

    formattedOutput.append(nString);
    
    if (dEdited && denominator != 1) {
	formattedOutput.append('/');
	if (denominator) 
	    formattedOutput.append(dString);
    }
    if (!formattedOutput.length())
	formattedOutput.append('0');
}
bool FractionData::del() {
    if (!edited)
        return TRUE;

    if (dEdited) {
	if (dString.length())  {
	    dString.truncate(dString.length()-1);
            bool ok = TRUE;
	    denominator=dString.toInt(&ok);
            if (!ok) {
                denominator = 1;
                dEdited = FALSE;
            }
	} else {
	    dEdited = FALSE;
	}
    } else {
        if (formattedOutput.length() == 1 || 
                (formattedOutput.length() == 2) && formattedOutput[0] == '-') {
            nString.truncate(0);
            nString.append("0");
            edited = FALSE;
            numerator = 0;
            return TRUE;
        } else 
	    nString.truncate(nString.length()-1);
    }

    buildFormattedString();
    return FALSE;
}
void FractionData::clear() {
    edited = FALSE;
    dEdited = FALSE;
    nString.truncate(0);
    dString.truncate(0);
    formattedOutput.truncate(0);
    formattedOutput.append("0");
    numerator = 0;
    denominator = 1;
    buildFormattedString();
}

void FractionData::set(int n,int d) {
    clear();
    if (!d)
	d = 1;
    int GCM = findGCM(n,d);
    numerator = n / GCM;
    denominator = d / GCM;
    if (d == 1) 
	dEdited = FALSE;
    else
	dEdited = TRUE;
    nString.setNum(numerator);
    dString.setNum(denominator);
    buildFormattedString();
}
// find greatest common d
int FractionData::doFindGCM ( int a, int b ) {
    int remainder = b % a;
    if ( remainder )
	return doFindGCM( remainder,a);
    return a;
}

// wrapper that makes sure that a and b
// are the right way round
int FractionData::findGCM ( int a, int b ) {
    if ( a == b )
	return a;
    if ( a > b )
	return doFindGCM ( a, b );
    else
	return doFindGCM ( b, a );
}
