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
#include "fractiondata.h"

// Data type functions
void FractionData::push(char c) {
    if (c == '/') {
	if (dEdited) 
	    return;
	denominator = 0;
	dEdited = TRUE;
	buildFormattedString();
	return;
    }

    // append char and test result
    bool ok = FALSE;
    QString tmpString;
    if (dEdited) {
	tmpString = dString;
	tmpString.append(c);
	denominator = tmpString.toInt(&ok);
	if (ok) {
	    dString = tmpString;
	}
    } else {
	tmpString = nString;
	tmpString.append(c);
	numerator = tmpString.toInt(&ok);
	if (ok) {
	    nString = tmpString;
	}
    }
    buildFormattedString();
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
    if (dEdited) {
	formattedOutput.append('/');
	if (denominator) 
	    formattedOutput.append(dString);
    }
    if (!formattedOutput.length())
	formattedOutput.append('0');
}
void FractionData::del() {
    bool ok = TRUE;

    if (dEdited) {
	if (dString.length())  {
	    dString.truncate(dString.length()-1);
	    denominator=dString.toInt(&ok);
	} else {
	    dEdited = FALSE;
	}
    } else {
	dEdited=FALSE;
	nString.truncate(nString.length()-1);
    }

    buildFormattedString();
}
void FractionData::clear() {
    dEdited = FALSE;
    nString.truncate(0);
    dString.truncate(0);
    formattedOutput.truncate(0);
    formattedOutput.append("0");
    numerator = 0;
    denominator = 1;
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
    nString.truncate(0);
    dString.truncate(0);
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
