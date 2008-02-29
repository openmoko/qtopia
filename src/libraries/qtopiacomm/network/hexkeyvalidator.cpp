/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "hexkeyvalidator.h"

#include <QString>

/*!
  \internal

  \class HexKeyValidator
  \brief The HexKeyValidator class validates hexadecimal keys such as WEP keys and MAC addresses.
  \mainclass

  This class should be used in connection with a QLineEdit. It allows the validation 
  of user input.
*/

HexKeyValidator::HexKeyValidator( QWidget* parent, int numDigits )
    : QValidator(parent) , neededNumDigits( numDigits )
{
}

QValidator::State HexKeyValidator::validate( QString& key, int& curs ) const
{
    QString k;
    int hexes=0;
    int ncurs=0;
    int digitCount = 0;
    for (int i=0; i<key.length(); i++) {
        char c=key[i].toUpper().toLatin1();
        if ( c>='0' && c<='9' || c>='A' && c<='F' ) {
            if ( hexes == 2 ) {
                hexes = 0;
                k += ':';
                if ( i<curs ) ncurs++;
            }
            k += c;
            digitCount++;
            hexes++;
            if ( i<curs ) ncurs++;
        } else if ( c == ':' && hexes==2 ) {
            hexes = 0;
            k += c;
            if ( i<curs ) ncurs++;
        } else {
            return Invalid;
        }
    }
    key = k;
    curs = ncurs;
    if ( neededNumDigits ) {
        if ( digitCount < neededNumDigits )
            return Intermediate;
        else if ( digitCount > neededNumDigits )
            return Invalid;
        //else
        //  return Acceptable
    }
    return Acceptable;
}


