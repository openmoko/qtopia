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
#ifndef CALC_FRACTIONDATA_H
#define CALC_FRACTIONDATA_H

#include "data.h"

class QTOPIA_EXPORT FractionData:public Data {
public:
    FractionData(){clear();};
    ~FractionData(){};
    QString getType(){return "FRACTION";};

    bool push(char,bool);
    void del();
    void clear();

    void set(int,int);
    int getNumerator(){return numerator;};
    int getDenominator(){return denominator;};
protected:
    virtual void buildFormattedString();
private:
    int findGCM(int,int);
    int doFindGCM(int,int);
    int numerator,denominator;
    bool dEdited;
    QString nString, dString;
};

#endif
