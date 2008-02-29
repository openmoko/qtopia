/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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
#include <qstring.h>

#include "doubledata.h"
#include "engine.h"
#include <QDebug>

// Data type
DoubleData::DoubleData(): Data() {};

/* This function returns \a number rounded using \a precision as precision indicator.
   The assumption is that the digit in position <code>precision+1</code> is used to work
   out what rounding rule to apply. This means the <code>precision+1</code>. digit in \a number
   must still be correct.
   */
QString roundDigits( const QString number, int precision ) {
    QString rounded = number;
    int point = number.indexOf( '.' ) ;
    if ( point > -1 && point <= precision-1 ) {
        bool up = (number.at(precision+1).isDigit() && number.at(precision+1) >= QChar('5')  );
        if ( !up ) {
            return rounded;
        }
        for ( int i = precision; i>= 0 ; i-- ) {
            if ( !up ) {
                return rounded;
            }
            QChar current = rounded.at(i);
            if ( !current.isDigit() ) {
                continue;
            }
            QChar newCurrent = (current == QChar('9')) ? QChar('0') : QChar(current.toAscii()+1) ;
            up = (newCurrent == QChar('0') );
            rounded = rounded.replace( i , 1,  newCurrent );
        }
    }
    return rounded;
}

void DoubleData::set(double d) {
    dbl = d;
    edited = false;
    formattedOutput = formattedOutput.sprintf("%13.13f", dbl);
    int point = formattedOutput.indexOf('.');
    if (point > 11)
    {
        QRegExp reg = QRegExp("[1..9]+");
        if (point > 12)
            systemEngine->setError(eSurpassLimits);
        else if ( point == 12 && formattedOutput.mid(13).contains(reg) ) {
            systemEngine->setError(eSurpassLimits);
        }
    }

    formattedOutput = roundDigits(formattedOutput, 12);
   
    if ( formattedOutput.at( 11 ) == QChar('.') && formattedOutput.at(12) != QChar('0') )
        systemEngine->setError( eSurpassLimits );
    formattedOutput.truncate(12);
    //remove trailing zeros if decimal point is present
    if (formattedOutput.indexOf('.') > -1){
        int max = formattedOutput.length();
        int i = max - 1;
        while (formattedOutput.at(i) == '0')
            i--;
        formattedOutput = formattedOutput.left( i+1 );

        if (formattedOutput.at( i ) == '.')
            formattedOutput.remove( i , 1 );
    }

    if (!strcmp(formattedOutput.toLatin1(),"nan")) { // No tr
        systemEngine->setError(eNotANumber);
    } else if (!strcmp(formattedOutput.toLatin1(),"inf")) { // No tr
        systemEngine->setError(eInf);
    } else if (!strcmp(formattedOutput.toLatin1(),"-inf")) { // No tr
        systemEngine->setError(eNegInf);
    }
}

double DoubleData::get() { return dbl; }

bool DoubleData::push(char c, bool commit) {
    if (edited && formattedOutput.length() >= 12)
        return false;
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
            edited = true;
    }
    tmpString.append(QChar(c));
    bool ok;
    double tmp = tmpString.toDouble(&ok);
    if (ok) {
        if (commit) {
            formattedOutput = tmpString;
            dbl = tmp;
        }
    }
    else {
        //qDebug("Wrong character pushed");
    }
    return ok;
}
bool DoubleData::del() {
    if (!edited)
        return true;
    if (formattedOutput.length() == 1) {
        formattedOutput.truncate(0);
        formattedOutput.append("0");
        edited = false;
        dbl = 0;
        return true;
    } else {
        QString tmpString = formattedOutput;
        tmpString.truncate(formattedOutput.length()-1);
        bool ok;
        double tmp = tmpString.toDouble(&ok);
        if (ok) {
            formattedOutput = tmpString;
            dbl = tmp;
        } else
            return true;
    }
    return false;
}
void DoubleData::clear() {
    dbl = 0;
    formattedOutput.truncate(0);
    formattedOutput.append("0");
    edited = false;
}
