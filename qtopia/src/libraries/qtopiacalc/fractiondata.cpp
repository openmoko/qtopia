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

#include <ctype.h>

static QString qlltoa(int64 l);
static int64 qstrtoll(const char *nptr, const char **endptr, bool *ok);

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
    int64 num;
    if (dEdited) {
	tmpString = dString;
	tmpString.append(c);
        const char *endptr;
        //save to use latin1() because the calculator can only handle ascii numbers
        num = qstrtoll(tmpString.latin1(), &endptr, &ok); 
        //num = tmpString.toInt(&ok);
	if (ok && *endptr == '\0' && commit) {
	    dString = tmpString;
	    denominator = num;
	}
    } else {
	tmpString = nString;
	tmpString.append(c);
        const char *endptr;
        num = qstrtoll(tmpString.latin1(), &endptr, &ok);
	//num = tmpString.toInt(&ok);
	if (ok && *endptr == '\0' && commit) {
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
	nString = qlltoa(numerator);
	denominator = 0 - denominator;
	dString.truncate(0);
	dString = qlltoa(denominator);
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
            const char *endptr;
            denominator=qstrtoll(dString.latin1(), &endptr, &ok);
	    //denominator=dString.toInt(&ok);
            if (!ok || *endptr != '\0') {
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
            numerator = numerator / 10;
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

void FractionData::set(int64 n, int64 d) {
    clear();
    if (!d)
	d = 1;
    int64 GCM = findGCM(n,d);
    numerator = n / GCM;
    denominator = d / GCM;
    if (d == 1) 
	dEdited = FALSE;
    else
	dEdited = TRUE;
    nString = qlltoa(numerator);
    dString = qlltoa(denominator);
    buildFormattedString();
}
// find greatest common d
int64 FractionData::doFindGCM ( int64 a, int64 b ) {
    int64 remainder = b % a;
    if ( remainder )
	return doFindGCM( remainder,a);
    return a;
}

// wrapper that makes sure that a and b
// are the right way round
int64 FractionData::findGCM ( int64 a, int64 b ) {
    if ( a == b )
	return a;
    if ( a > b )
	return doFindGCM ( a, b );
    else
	return doFindGCM ( b, a );
}

/* qulltoa(..) and qlltoa(...) copied from Qt4's qlocale.cpp */

static QString qulltoa(int64 l)
{
    QChar buff[20]; // length of int64 in base 10
    QChar *p = buff + 20;
    
    int zero = QChar('0').unicode();
    
    while (l != 0) {
        int c = l % 10;
        *(--p) = zero + c;
        l /= 10;
    }

    return QString(p, 20 - (p - buff));
}

static QString qlltoa(int64 l)
{
    if (l >= 0)
        return qulltoa(l);
    else
        return qulltoa(-l).prepend(QChar('-'));
}



typedef unsigned long long uint64; 
#define isascii(c) ((int)( (unsigned int) (c) <= (unsigned char)0x7f))

//  "$FreeBSD: src/lib/libc/stdlib/strtoll.c,v 1.5.2.1 2001/03/02 09:45:20 obrien Exp $";
//  Qt4: src/core/tools/qlocale.cpp

/*
 * Convert a string to a qint64 integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
static int64 qstrtoll(const char *nptr, const char **endptr, bool *ok)
{
    register const char *s;
    register uint64 acc;
    register unsigned char c;
    register uint64 qbase, cutoff;
    register int neg, any, cutlim;

    //we define our own 12 digits - we can't display more digits anyway
    const int64 CALC_MAX = 999999999999LL;
    const int64 CALC_MIN = -CALC_MAX-1LL;
    
    if (ok != 0)
        *ok = true;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    s = nptr;
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for quads is
     * [-9223372036854775808..9223372036854775807] and the input base
     * is 10, cutoff will be set to 922337203685477580 and cutlim to
     * either 7 (neg==0) or 8 (neg==1), meaning that if we have
     * accumulated a value > 922337203685477580, or equal but the
     * next digit is > 7 (or 8), the number is too big, and we will
     * return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    qbase = (unsigned)(10);
    cutoff = neg ? uint64(0-(CALC_MAX + CALC_MAX)) + CALC_MAX : CALC_MAX;
    cutlim = cutoff % qbase;
    cutoff /= qbase;
    for (acc = 0, any = 0;; c = *s++) {
        if (!isascii(c))
            break;
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= 10)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= qbase;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? CALC_MIN : CALC_MAX;
        if (ok != 0)
            *ok = false;
    } else if (neg) {
        acc = (~acc) + 1;
    }
    if (endptr != 0)
        *endptr = (any ? s - 1 : nptr);
    return (acc);
}


